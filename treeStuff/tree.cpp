#include "tree.hpp"
#include "glm/gtx/string_cast.hpp"

void Tree::getBudPositions(std::vector<glm::vec3>& budVector, const Metamer* metamer) {
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


void Tree::updateHarvestedLight(Metamer* internode, World* world) {
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


void Tree::distributeHarvestedLight(Metamer* internode, double inboundLight) {
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


void printVec3(glm::vec3 vec) {
    std::cout << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}" << std::endl;
}


void Tree::growShoots(Metamer* internode, World* world) {
    //if vigor > shedding grow a new metamer ?
    if(internode->mainAxis) {
        growShoots(internode->mainAxis, world);
    }

    if(internode->latAxis) {
        growShoots(internode->latAxis, world);
    }

    if(!internode->has_main_aborted) {
        if(!internode->mainAxis && internode->mainAxisLight > branch_shedding_threshold) {
            //add shoot to main
            glm::vec3 ideal_branch_angle = getIdealBranchAngle(internode, world, 0);
            
            Metamer* new_branch = (Metamer*)malloc(sizeof(Metamer));
            memset(new_branch, 0, sizeof(Metamer));
            new_branch->base = internode->end;
            new_branch->end = internode->end + ideal_branch_angle;
            internode->mainAxis = new_branch;
        } else if(internode->mainAxis && internode->mainAxisLight < branch_shedding_threshold) {
            destroyMetamer(internode->mainAxis);
            internode->has_main_aborted = true;
            internode->mainAxis = 0;
        }
    }

    if(!internode->has_lat_aborted) {
        if(!internode->latAxis && internode->latAxisLight > branch_shedding_threshold) {
            //add shoot to lat
            glm::vec3 ideal_branch_angle = getIdealBranchAngle(internode, world, 1);
            Metamer* new_branch = (Metamer*)malloc(sizeof(Metamer));
            memset(new_branch, 0, sizeof(Metamer));
            new_branch->base = internode->end;
            new_branch->end = internode->end + ideal_branch_angle;

            internode->latAxis = new_branch;
        } else if(internode->latAxis && internode->latAxisLight < branch_shedding_threshold) {
            destroyMetamer(internode->latAxis);
            internode->has_lat_aborted = true;
            internode->latAxis = 0;
        }
    }
}


// axis should be 0 if getting branch angle for main, 1 if lateral
glm::vec3 Tree::getIdealBranchAngle(Metamer* internode, World* world, size_t axis) {
    //Get a vector normal to direction of main branch
    glm::vec3 norm_main_axis = glm::normalize(internode->end - internode->base);
    glm::vec3 a;
    //The above line does not check for 0 value of x
    if(norm_main_axis.x != 0) {
        float a1 = (-norm_main_axis.y - norm_main_axis.z)/(norm_main_axis.x);
        a = glm::vec3{a1, 1, 1};
    } else if (norm_main_axis.y != 0) {
        float a1 = (-norm_main_axis.x - norm_main_axis.z)/(norm_main_axis.y);
        a = glm::vec3{1, a1, 1};
    } else {
        float a1 = (-norm_main_axis.y - norm_main_axis.x)/(norm_main_axis.z);
        a = glm::vec3{1, 1, a1};
    }
    a = glm::normalize(a);

    glm::vec3 default_angle;
    if(axis) { // 0 if main axis, 1 if lateral axis
        // TODO check is easier to do something like def_ang = {default_length cos B, default length sin B, ...}
        //rotate by branching angle with normal a
        glm::mat4 rot_matrix(1);

        //rot_matrix = glm::rotate(rot_matrix, glm::radians(branching_angle), a);
        rot_matrix = glm::rotate(rot_matrix, branching_angle, a);
        
        default_angle = glm::vec3(rot_matrix * glm::vec4(norm_main_axis, 1.0f));
        //rotate random amount along main axis (end - base)
        glm::mat4 rot_matrix2(1);
        
        // TODO add variable to control random rotation relates to phyllotaxis
        // ex. spiral pattern with rotations for different trees
        float rand_rot = rand() % 360;

        rot_matrix2 = glm::rotate(rot_matrix2, rand_rot, norm_main_axis);
        
        default_angle = glm::vec3(rot_matrix2 * glm::vec4(default_angle, 1.0f));
    } else {
        default_angle = norm_main_axis;
    }
    default_angle = glm::normalize(default_angle);

    // optimal angle will be voxel in perception area with least shade
    glm::vec3 optimal_growth_dir = world->getOptimalGrowthDirection(internode->end, default_angle);

    // tropism vector will just be up or down
    glm::vec3 tropism_vector = glm::vec3{0, 0, tropism_up_or_down};

    // ideal angle = default + eps*optimal_growth_vector + eta*tropism_vector
    glm::vec3 ideal = default_angle + epsilon * optimal_growth_dir + eta * tropism_vector;
    ideal = DEFAULT_START_LENGTH * glm::normalize(ideal);

    return ideal;
}


// TODO probably better to make this a pipe at some point in the future
//
// change createExportList to maintain record of index value, also return number of 
// vertices visited in a call so you can accurately update index
size_t Tree::createExportList(std::fstream& vfile, std::fstream& ifile, Metamer* internode, size_t index) {
    vfile.write((const char*)(&internode->end.x), sizeof(float));
    vfile.write((const char*)(&internode->end.y), sizeof(float));
    vfile.write((const char*)(&internode->end.z), sizeof(float));

    // write index value to a separate file here
    ifile.write((const char*)(&index), sizeof(size_t));

    printVec3(internode->end);
    std::cout << " , index: " << index << std::endl;

    size_t visited = 0;
    size_t restart = 0xFFFFFFFF;
    if(internode->mainAxis) {
        visited = createExportList(vfile, ifile, internode->mainAxis, index + 1);
        // write restart index
        ifile.write((const char*)(&restart), sizeof(size_t));        
        // write index value to a separate file here
        ifile.write((const char*)(&index), sizeof(size_t));
    }

    if(internode->latAxis) {
        visited += createExportList(vfile, ifile, internode->latAxis, index + 1 + visited);
    }

    return 1+visited;
}


void Tree::printMetamerTree(Metamer* metamer) {
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


void Tree::destroyMetamer(Metamer* metamer) {
    if(metamer->latAxis) {
        destroyMetamer(metamer->latAxis);
    }

    if(metamer->mainAxis) {
        destroyMetamer(metamer->mainAxis);
    }

    free(metamer);
}


//////////////////////////
// PUBLIC FUNCTIONS BELOW
//////////////////////////


// branching angle should be in degrees and is converted to radians
// Might want to make an info struct to make the parameter list shorter
Tree::Tree(float apical, float det, float angle, float res_distr, float max_vigor, float shedding, float x, float y, float z) {
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
    
    // TODO not hard code these
    this->tropism_up_or_down = -1;
    this->epsilon = 1.0f;
    this->eta = 1.0f;
}


std::vector<glm::vec3> Tree::getTreeBudLocations() {
    std::vector<glm::vec3> budPositions;
    getBudPositions(budPositions, root);

    return budPositions;
}


void Tree::distributeLight(World* world) {
    updateHarvestedLight(root, world);
    distributeHarvestedLight(root, (root->mainAxisLight + root->latAxisLight));
}


void Tree::growNewShoots(World* world) {
    growShoots(root, world);
}


void Tree::printTree() {
    printMetamerTree(this->root);
}


void Tree::exportPoints(std::fstream& vert_file, std::fstream& index_file) {
    vert_file.write((const char*)(&root->base.x), sizeof(float));
    vert_file.write((const char*)(&root->base.y), sizeof(float));
    vert_file.write((const char*)(&root->base.z), sizeof(float));

    //write index value here which would be 0
    size_t i = 0;
    index_file.write((const char*)(&i), sizeof(size_t));

    printVec3(root->base);
    std::cout << " , index: 0" << std::endl;

    createExportList(vert_file, index_file, root, i+1);
}


Tree::~Tree() {
    destroyMetamer(this->root);
}
