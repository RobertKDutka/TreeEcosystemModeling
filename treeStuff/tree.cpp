#include "tree.hpp"


// branching angle should be in degrees and is converted to radians
// Might want to make an info struct to make the parameter list shorter
World::Tree(float apical, float det, float angle, float res_distr, float max_vigor, float shedding, float x, float y, float z) {
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


std::vector<glm::vec3> World::getTreeBudLocations() {
    std::vector<glm::vec3> budPositions;
    getBudPositions(budPositions, root);

    return budPositions;
}


void World::distributeLight(World* world) {
    updateHarvestedLight(root, world);
    distributeHarvestedLight(root, (root->mainAxisLight + root->latAxisLight));
}


void World::growNewShoots(World* world) {
    growShoots(root, world);
}


void World::printTree() {
    printMetamerTree(this->root);
}


World::~Tree() {
    destroyMetamer(this->root);
}


void World::getBudPositions(std::vector<glm::vec3>& budVector, const Metamer* metamer) {
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


void World::updateHarvestedLight(Metamer* internode, World* world) {
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


void World::distributeHarvestedLight(Metamer* internode, double inboundLight) {
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


void World::growShoots(Metamer* internode, World* world) {
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
            glm::vec3 ideal_branch_angle = getIdealBranchAngle(internode, world, 0);
            
            Metamer* new_branch = (Metamer*)malloc(sizeof(Metamer));
            memset(new_branch, 0, sizeof(Metamer));
            new_branch->base = internode->end;
            new_branch->end = internode->end + ideal_branch_angle;
            internode->mainAxis = new_branch;

        } else if(internode->mainAxisLight < branch_shedding_threshold) {
            destroyMetamer(internode->mainAxis);
            internode->has_main_aborted = true;
        }
    }

    if(!internode->has_lat_aborted) {
        if(internode->latAxisLight > branch_shedding_threshold) {
            //add shoot to lat
            glm::vec3 ideal_branch_angle = getIdealBranchAngle(internode, world, 1);
            
            Metamer* new_branch = (Metamer*)malloc(sizeof(Metamer));
            memset(new_branch, 0, sizeof(Metamer));
            new_branch->base = internode->end;
            new_branch->end = internode->end + ideal_branch_angle;

            internode->latAxis = new_branch;
        } else if(internode->latAxisLight < branch_shedding_threshold) {
            destroyMetamer(internode->latAxis);
            internode->has_lat_aborted = true;
        }
    }
}


// axis should be 0 if getting branch angle for main, 1 if lateral
glm::vec3 World::getIdealBranchAngle(Metamer* internode, World* world, size_t axis) {
    //Get a vector normal to direction of main branch
    glm::vec3 norm_main_axis = glm::normalize(internode->end - internode->base);
    glm::vec3 a{(-norm_main_axis.y - norm_main_axis.z) / norm_main_axis.x, 1, 1};
    a = glm::normalize(a);

    glm::vec3 default_angle;
    if(axis) { // 0 if main axis, 1 if lateral axis
        //rotate by branching angle with normal a
        glm::mat4 rot_matrix(1);
        rot_matrix = glm::rotate(rot_matrix, glm::radians(branching_angle), a);
        default_angle = glm::vec3(rot_matrix * glm::vec4(norm_main_axis, 1.0f));
        //rotate random amount along main axis (end - base)
        glm::mat4 rot_matrix(1);
        float rand_rot = rand() % 360;
        rot_matrix = glm::rotate(rot_matrix, glm::radians(rand_rot), norm_main_axis);
        default_angle = glm::vec3(rot_matrix * glm::vec4(default_angle, 1.0f));
    } else {
        default_angle = norm_main_axis;
    }

    // optimal angle will be voxel in perception area with least shade
    glm::vec3 optimal_growth_dir = world->getOptimalGrowthDirection(internode->end, default_angle);
    
    // tropism vector will just be up or down
    glm::vec3 tropism_vector = glm::vec3{0, tropism_up_or_down, 0};

    // ideal angle = default + eps*optimal_growth_vector + eta*tropism_vector
    glm::vec3 ideal = default_angle + epsilon * optimal_growth_dir + eta * tropism_vector;
    ideal = DEFAULT_START_LENGTH * glm::normalize(ideal);

    return ideal;
}


void World::printMetamerTree(Metamer* metamer) {
    std::cout << "Metamer Base: ";
    printVec3(metamer->base);
    std::cout << "\tEnd: ";
    printVec3(metamer->end);
    std::cout << std::endl;

    if(metamer->mainAxis) {
        std::cout << "mainAxis: ";
        printMetamerTree(metamer->mainAxis);
    }

    if(metamer->latAxis) {
        std::cout << "latAxis: ";
        printMetamerTree(metamer->latAxis);
    }
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


void printVec3(glm::vec3 vec) {
    std::cout << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
}