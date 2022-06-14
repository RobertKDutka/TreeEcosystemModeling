#pragma once

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "world.hpp"


class World;
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

        void getBudPositions(std::vector<glm::vec3>& budVector, const Metamer* metamer);

        void updateHarvestedLight(Metamer* internode, World* world);

        void distributeHarvestedLight(Metamer* internode, double inboundLight);

        void growShoots(Metamer* internode, World* world);

        glm::vec3 getIdealBranchAngle(Metamer* internode, World* world, size_t axis);

        void printMetamerTree(Metamer* metamer);

        void destroyMetamer(Metamer* metamer);
    public:
        Tree(float apical, float det, float angle, float res_distr, float max_vigor, float shedding, float x, float y, float z);

        std::vector<glm::vec3> getTreeBudLocations();

        void distributeLight(World* world);

        void growNewShoots(World* world);

        void printTree();

        ~Tree();


};