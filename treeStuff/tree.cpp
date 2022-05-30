#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include "world.cpp"

extern const size_t VOXEL_GRID_LENGTH;
const float DEFAULT_START_LENGTH = 0.5;

// Metamer count per tree in the hundreds of thousands for old trees
struct Metamer {
    glm::vec3 base;
    glm::vec3 end;

    double mainAxisLight;
    double latAxisLight;
    
    bool has_main_aborted;
    bool has_lat_aborted;

    bool has_main_flowered;
    bool has_lat_flowered;

    Metamer* mainAxis;
    Metamer* latAxis;
};

class Tree {
    public:
        Tree();

        // branching angle should be in degrees and is converted to radians
        // Might want to make an info struct to make the parameter list shorter
        Tree(float apical, float det, float angle, float res_distr, float max_vigor, float shedding, float x, float y, float z) {
            Metamer* new_root = (Metamer*)malloc(sizeof(Metamer));
            memset(new_root, 0, sizeof(Metamer));
            new_root->base = glm::vec3{x, y, z};
            new_root->end = glm::vec3{x, y, z + DEFAULT_START_LENGTH};

            this->root = new_root;

            this->apical_control = apical;
            this->determinacy = det;
            this->branching_angle = glm::radians(angle);
            this->resource_distribution_coefficient = res_distr;
            this->root_vigor_max = max_vigor;
            this->branch_shedding_threshold = shedding;
        }

        std::vector<glm::vec3> getTreeBudLocations() {
            std::vector<glm::vec3> budPositions;
            getBudPositions(budPositions, root);

            return budPositions;
        }

        void distributeLight(World* world) {
            updateHarvestedLight(root, world);
            distributeHarvestedLight(root, (root->mainAxisLight + root->latAxisLight));
        }

        void growNewShoots(World* world) {
            growShoots(root, world);
        }

        ~Tree() {
            destroyMetamer(this->root);
        }

    private:
        Metamer* root;
        float apical_control; // [0, 1]
        float determinacy; // [0, 1]
        float branching_angle; // [0, 180] will be converted to radians
        float resource_distribution_coefficient; // Typical ~=2
        float root_vigor_max; // have to guess this :) samples values in one paper i guess
        float branch_shedding_threshold; // have to guess this :)
        float epsilon; // growth direction weight. have to guess this :)
        float eta; // tropism direction weight. have to guess this :)
        float tropism_up_or_down; // [-1, 1]. will be a vec3 {0, tuod, 0}

        void getBudPositions(std::vector<glm::vec3>& budVector, const Metamer* metamer) {
            // If nothing past terminal bud add terminal bud.
            // Else get buds up main axis
            if(!(metamer->mainAxis) && !metamer->has_main_aborted) {
                budVector.push_back(metamer->end);
            } else if(metamer->mainAxis){
                getBudPositions(budVector, metamer->mainAxis);
            }
            
            // If no lateral branch and not added terminal bud add axillary bud
            // Else add buds up lateral branch
            if(!(metamer->latAxis) && !metamer->has_lat_aborted) {
                budVector.push_back(metamer->end);
            } else if(metamer->latAxis) {
                getBudPositions(budVector, metamer->latAxis);
            }
        }

        void updateHarvestedLight(Metamer* internode, World* world) {
            internode->mainAxisLight = 0;
            internode->latAxisLight = 0;

            double mainLight = 0;
            if(internode->mainAxis) {
                updateHarvestedLight(internode->mainAxis, world);
                mainLight = internode->mainAxis->mainAxisLight + internode->mainAxis->latAxisLight;
            } else if(!internode->has_main_aborted){
                mainLight = world->getLightAtVoxel(internode->end);
            }
            internode->mainAxisLight = mainLight;

            double latLight = 0;
            if(internode->latAxis) {
                updateHarvestedLight(internode->latAxis, world);
                latLight = internode->latAxis->mainAxisLight + internode->latAxis->latAxisLight;
            } else if(!internode->has_lat_aborted) {
                latLight = world->getLightAtVoxel(internode->end);
            }
            internode->latAxisLight = latLight;
        }

        void distributeHarvestedLight(Metamer* internode, double inboundLight) {
            if(internode->has_main_aborted) {
                internode->mainAxisLight = 0;
                internode->latAxisLight = inboundLight;
            } else if(internode->has_lat_aborted) {
                internode->mainAxisLight = inboundLight;
                internode->latAxisLight = 0;
            } else {
                double mainWeight = inboundLight * apical_control * internode->mainAxisLight;
                double bottomWeight = apical_control * internode->mainAxisLight + (1-apical_control)*internode->latAxisLight;
                
                internode->mainAxisLight = mainWeight / bottomWeight;
                internode->latAxisLight = inboundLight - internode->mainAxisLight;
            }
            
            if(internode->mainAxis) {
                distributeHarvestedLight(internode->mainAxis, internode->mainAxisLight);
            }

            if(internode->latAxis) {
                distributeHarvestedLight(internode->latAxis, internode->latAxisLight);
            }
        }

        void growShoots(Metamer* internode, World* world) {
            //if vigor > shedding grow a new metamer ?
            if(internode->mainAxis) {
                growShoots(internode->mainAxis, world);
            }

            if(internode->latAxis) {
                growShoots(internode->latAxis, world);
            }

            if(!internode->has_main_aborted) {
                if(internode->mainAxisLight > branch_shedding_threshold) {
                    //add shoot to main
                    glm::vec3 ideal_branch_angle = getIdealBranchAngle(internode, world);
                    Metamer* new_branch = (Metamer*)malloc(sizeof(Metamer));
                    memset(new_branch, 0, sizeof(Metamer));
                    new_branch->base = internode->end;
                    new_branch->end = internode->end + ideal_branch_angle;
                }
            }

            if(!internode->has_lat_aborted) {
                if(internode->latAxisLight > branch_shedding_threshold) {
                    //add shoot to lat
                }
            }
        }

        glm::vec3 getIdealBranchAngle(Metamer* internode, World* world) {
            //Get a vector normal to direction of main branch
            glm::vec3 norm_main_axis = glm::normalize(internode->end - internode->base);
            glm::vec3 a{(-norm_main_axis.y - norm_main_axis.z) / norm_main_axis.x, 1, 1};
            a = glm::normalize(a);

            //rotate by branching angle with normal a
            glm::mat4 rot_matrix(1);
            rot_matrix = glm::rotate(rot_matrix, glm::radians(branching_angle), a);
            glm::vec3 default_angle = glm::vec3(rot_matrix * glm::vec4(norm_main_axis, 1.0f));
            //rotate random amount along main axis (end - base)
            glm::mat4 rot_matrix(1);
            float rand_rot = rand() % 360;
            rot_matrix = glm::rotate(rot_matrix, glm::radians(rand_rot), norm_main_axis);
            default_angle = glm::vec3(rot_matrix * glm::vec4(default_angle, 1.0f));

            // optimal angle will be voxel in perception area with least shade
            glm::vec3 optimal_growth_dir = world->getOptimalGrowthDirection(internode->end, default_angle);
            
            // tropism vector will just be up or down
            glm::vec3 tropism_vector = glm::vec3{0, tropism_up_or_down, 0};

            // ideal angle = default + eps*optimal_growth_vector + eta*tropism_vector
            glm::vec3 ideal = default_angle + epsilon * optimal_growth_dir + eta * tropism_vector;
            ideal = DEFAULT_START_LENGTH * glm::normalize(ideal);

            return ideal;
        }

        void destroyMetamer(Metamer* metamer) {
            if(metamer->latAxis) {
                destroyMetamer(metamer->latAxis);
            }

            if(metamer->mainAxis) {
                destroyMetamer(metamer->mainAxis);
            }

            free(metamer);
        }
};