#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <fstream>

#include "tree.hpp"

class Tree;

const size_t VOXEL_GRID_LENGTH = 201; // 200x200x200
const size_t VOXEL_CUBIC_AREA = 201*201*201;
const size_t SHADOW_DEPTH = 6; // Typically 4-8
const float MAX_LIGHT_EXPOSURE = 4.0f; //IDK they dont specify time to try stuff out

const float THETA = 90.0f; // typically 90 deg; pi/2 radians
const float PERCEPTION_RANGE = 4.0f; // typically 4-6 internode lengths


class World {
    private:
        Tree* tree;

        // Shadow propagation model
        float voxels[VOXEL_GRID_LENGTH][VOXEL_GRID_LENGTH][VOXEL_GRID_LENGTH];
        float a; // a > 0
        float b; // b > 1
        
        void updateShade(glm::vec3 bud);

        void updateWorldState();

        void distributeResources();

        void appendNewShoots();

        float getMaxCoordinate(glm::vec3 circle_norm, glm::vec3 circle_center, glm::vec3 dir, float radius);

        void getMaxCoordinates2(glm::vec3 cone_tip, glm::vec3 circ_cent, glm::vec3 circ_norm, float radius, float* max_vals);

        glm::vec3 project_dir_to_circle_plane(glm::vec3 circle_norm, glm::vec3 circle_center, glm::vec3 dir, float radius);

        float round_down_near_half(float num);

        float round_up_near_half(float num);

        bool is_point_in_cone(glm::vec3 cone_tip, glm::vec3 tip_to_base, float radius, glm::vec3 point);

        /*
        // Space colonization model
        size_t propagation_grid_length; //10x10x10
        size_t marker_density;
        float rho; // typically 2 internode lengths
        float theta; // typically 90 deg; pi/2 radians
        float perception_range; // typically 4-6 internode lengths
        */
    
    public:
        World(float a, float b, float apical, float det, float angle, float res_distr, float max_vigor, float shedding);

        ~World();

        /*
            Seedling Structure
                    |
            ------->V
            |   calc local bud environment
            |   determine bud fate
            |   append new shoots
            |   shed branches
            |   update branch width
            L----<--|
                    V
                growing tree

        */
       
        void runTimeStep();

        float getLightAtVoxel(glm::vec3 pos);

        glm::vec3 getOptimalGrowthDirection(glm::vec3 point, glm::vec3 search_dir);

        void printTrees();

        void exportPoints();
};