#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string.h>
#include <algorithm>
#include <cmath>
#include <limits>

#include "tree.cpp"

const size_t VOXEL_GRID_LENGTH = 201; // 200x200x200
const size_t VOXEL_CUBIC_AREA = 201*201*201;
const size_t SHADOW_DEPTH = 6; // Typically 4-8
const float MAX_LIGHT_EXPOSURE = 1.0f; //IDK they dont specify time to try stuff out

const float THETA = 90.0f; // typically 90 deg; pi/2 radians
const float PERCEPTION_RANGE = 4.0f; // typically 4-6 internode lengths

class World {
    public:
        World(float a, float b, float apical, float det, float angle, float res_distr, float max_vigor, float shedding) {
            memset(voxels, 0, VOXEL_CUBIC_AREA);
            this->a = a;
            this->b = b;

            this->tree = new Tree(apical, det, angle, res_distr, max_vigor, shedding, VOXEL_GRID_LENGTH/2, VOXEL_GRID_LENGTH/2, 0);
        }

        ~World() {
            delete this->tree;
        }

        float getLightAtVoxel(glm::vec3 pos) {
            size_t x_box = static_cast<int>(pos.x);
            size_t y_box = static_cast<int>(pos.y);
            size_t z_box = static_cast<int>(pos.z);
            float light_minus_shade = (MAX_LIGHT_EXPOSURE - voxels[x_box][y_box][z_box]);
            return std::max(light_minus_shade, 0.0f);
        }

        // Find unit vector to voxel with most light,
        // from end point in a conical area in the search direction
        glm::vec3 getOptimalGrowthDirection(glm::vec3 point, glm::vec3 search_dir) {
            glm::vec3 circle_center = point + glm::normalize(search_dir) * PERCEPTION_RANGE;
            float radius = PERCEPTION_RANGE * glm::sin(glm::radians(THETA / 2));
            // Generate rectangular prism that encloses cone
            glm::vec3 pos_z_proj = project_dir_to_circle_plane(search_dir, circle_center, glm::vec3{0, 0, 1}, radius);
            glm::vec3 neg_z_proj = project_dir_to_circle_plane(search_dir, circle_center, glm::vec3{0, 0, -1}, radius);
            glm::vec3 pos_y_proj = project_dir_to_circle_plane(search_dir, circle_center, glm::vec3{0, 1, 0}, radius);
            glm::vec3 neg_y_proj = project_dir_to_circle_plane(search_dir, circle_center, glm::vec3{0, -1, 0}, radius);
            glm::vec3 pos_x_proj = project_dir_to_circle_plane(search_dir, circle_center, glm::vec3{1, 0, 0}, radius);
            glm::vec3 neg_x_proj = project_dir_to_circle_plane(search_dir, circle_center, glm::vec3{-1, 0, 0}, radius);

            float max_z = round_down_near_half(std::max(pos_z_proj.z, point.z));
            float min_z = round_up_near_half(std::min(neg_z_proj.z, point.z));
            float max_y = round_down_near_half(std::max(pos_y_proj.y, point.y));
            float min_y = round_up_near_half(std::min(neg_y_proj.y, point.y));
            float max_x = round_down_near_half(std::max(pos_x_proj.x, point.x));
            float min_x = round_up_near_half(std::min(neg_x_proj.x, point.x));

            // Test center points of voxels to determine if they are within cone
            float brightest_val = 0;
            std::vector<glm::vec3> ideal_points;
            for(float z = max_z; z > min_z; z-=1) {
                for(float y = max_y; y > min_y; y-=1) {
                    for(float x = max_x; x > min_x; x-=1) {
                        if(is_point_in_cone(point, search_dir, radius, glm::vec3{x, y, z})) {
                            float light_at_voxel = getLightAtVoxel(glm::vec3{x, y, z});
                            if(light_at_voxel == brightest_val) {
                                ideal_points.push_back(glm::vec3{x, y, z} - point);
                            } else if(light_at_voxel > brightest_val) {
                                brightest_val = light_at_voxel;
                                ideal_points.clear();
                                ideal_points.push_back(glm::vec3{x, y, z} - point);
                            }
                        }
                    }
                }
            }

            // Calculate normalized sum of normalized vectors to best points
            glm::vec3 ideal_vector = glm::normalize(ideal_points[0]);
            for(size_t i = 1; i < ideal_points.size(); i++) {
                ideal_vector += glm::normalize(ideal_points[i]);
            }
            ideal_vector = glm::normalize(ideal_vector);

            return ideal_vector;
        }

        void printTrees() {
            tree->printTree();
        }

    private:
        Tree* tree;

        // Shadow propagation model
        float voxels[VOXEL_GRID_LENGTH][VOXEL_GRID_LENGTH][VOXEL_GRID_LENGTH];
        float a; // a > 0
        float b; // b > 1
        
        // I could try to batch up the buds and then update the shadow value
        void updateShade(glm::vec3 bud) {
            size_t x_pos = static_cast<int>(bud.x);
            size_t y_pos = static_cast<int>(bud.y);
            size_t z_pos = static_cast<int>(bud.z);

            for(size_t q = 0; q <= SHADOW_DEPTH; q++) {
                float shadow_value = a * pow(b, -q);
                for(size_t px = 0; px < (1+2*q); px++) {
                    for(size_t py = 0; py < (1+2*q); py++) {
                        voxels[x_pos - q + px][y_pos - q + py][z_pos - q] += shadow_value;
                    }
                }
            }
        }

        
        
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
        void runTimeStep(){
            updateWorldState();
            distributeResources();
            appendNewShoots();
        }

        void updateWorldState() {
            // Reset shadow values
            memset(voxels, 0, VOXEL_CUBIC_AREA);
            //get bud positions and update shading per bud
            std::vector<glm::vec3> budLocations = tree->getTreeBudLocations();
            for(glm::vec3 bud : budLocations) {
                // Could possibly batch up bud locations to do fewer updates
                updateShade(bud);
            }
        }

        void distributeResources() {
            tree->distributeLight(this);
        }

        void appendNewShoots() {
            tree->growNewShoots(this);
        }

        glm::vec3 project_dir_to_circle_plane(glm::vec3 circle_norm, glm::vec3 circle_center, glm::vec3 dir, float  radius) {
            glm::vec3 dir_cross_norm = glm::cross(circle_norm, dir);
            glm::mat4 rot_matrix(1);
            // Might be a problem if the cross product is 0
            rot_matrix = glm::rotate(rot_matrix, glm::radians(90.0f), dir_cross_norm);
            glm::vec3 proj_vect = glm::vec3(rot_matrix * glm::vec4(circle_norm, 1.0f));
            proj_vect = glm::normalize(proj_vect) * radius;

            return (proj_vect + circle_center);
        }

        float round_down_near_half(float num) {
            float dec = num - static_cast<int>(num);
            if(dec > 0.5f) {
                dec -= 0.5f;
            }

            num -= dec;

            if(fmod(num*10, 10) != 5) {
                num -= 0.5f;
            }

            return num;
        }

        float round_up_near_half(float num) {
            float dec = num - static_cast<int>(num);
            if(dec > 0.5f) {
                dec -= 0.5f;
            }
            dec = 0.5f - dec;
            num += dec;

            if(fmod(num*10, 10) != 5) {
                num += 0.5f;
            }

            return num;
        }

        bool is_point_in_cone(glm::vec3 cone_tip, glm::vec3 tip_to_base, float radius, glm::vec3 point) {
            glm::vec3 norm_vec_2_base = glm::normalize(tip_to_base);
            glm::vec3 p_min_tip = point - cone_tip;
            float cone_dist = glm::dot(p_min_tip, norm_vec_2_base);

            // Dismiss points not between tip and height of cone
            if(0 > cone_dist || cone_dist > PERCEPTION_RANGE) { return false; }

            //Check if point is within interpolated radius
            float radius_at_height = (cone_dist / PERCEPTION_RANGE) * radius;
            float dist_to_center = glm::length(p_min_tip - cone_dist*norm_vec_2_base);

            if(dist_to_center > radius_at_height) {
                return false;
            } else {
                return true;
            }
        }

        /*
        // Space colonization model
        size_t propagation_grid_length; //10x10x10
        size_t marker_density;
        float rho; // typically 2 internode lengths
        float theta; // typically 90 deg; pi/2 radians
        float perception_range; // typically 4-6 internode lengths
        */
    
};