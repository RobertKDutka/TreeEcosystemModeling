#include "world.hpp"


World::World(float a, float b, float apical, float det, float angle, float res_distr, float max_vigor, float shedding) {
    memset(voxels, 0, VOXEL_CUBIC_AREA);
    this->a = a;
    this->b = b;

    this->tree = new Tree(apical, det, angle, res_distr, max_vigor, shedding, VOXEL_GRID_LENGTH/2, VOXEL_GRID_LENGTH/2, 0);
}


World::~World() {
    delete this->tree;
}


float World::getLightAtVoxel(glm::vec3 pos) {
    size_t x_box = static_cast<int>(pos.x);
    size_t y_box = static_cast<int>(pos.y);
    size_t z_box = static_cast<int>(pos.z);
    float light_minus_shade = (MAX_LIGHT_EXPOSURE - voxels[x_box][y_box][z_box] + a);
    return std::min(std::max(light_minus_shade, 0.0f), MAX_LIGHT_EXPOSURE);
}


// Find unit vector to voxel with most light,
// from end point in a conical area in the search direction
glm::vec3 World::getOptimalGrowthDirection(glm::vec3 point, glm::vec3 search_dir) {
    glm::vec3 circle_center = point + glm::normalize(search_dir) * PERCEPTION_RANGE;
    float radius = PERCEPTION_RANGE * glm::tan(glm::radians(THETA / 2.));
    
    // float min_x = getMaxCoordinate(search_dir, circle_center, glm::vec3{-1, 0, 0}, radius);
    // float max_x = getMaxCoordinate(search_dir, circle_center, glm::vec3{1, 0, 0}, radius);
    // float min_y = getMaxCoordinate(search_dir, circle_center, glm::vec3{0, -1, 0}, radius);
    // float max_y = getMaxCoordinate(search_dir, circle_center, glm::vec3{0, 1, 0}, radius);
    // float min_z = getMaxCoordinate(search_dir, circle_center, glm::vec3{0, 0, -1}, radius);
    // float max_z = getMaxCoordinate(search_dir, circle_center, glm::vec3{0, 0, 1}, radius);
    float bounds[6]; // 0 is min x, 1 is max x, 2 is min y 3 is max y, 4 is min z 5 is max z
    getMaxCoordinates2(point, circle_center, search_dir, radius, bounds);
    

    // Test center points of voxels to determine if they are within cone
    float brightest_val = 0;
    std::vector<glm::vec3> ideal_points;
    for(float z = bounds[5]; z > bounds[4]; z-=1) {
        for(float y = bounds[3]; y > bounds[2]; y-=1) {
            for(float x = bounds[1]; x > bounds[0]; x-=1) {
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


void World::getMaxCoordinates2(glm::vec3 cone_tip, glm::vec3 circ_cent, glm::vec3 circ_norm, float radius, float* max_vals) {
    circ_norm = glm::normalize(circ_norm);
    bool parallel_x_plane = false;
    bool parallel_y_plane = false;
    bool parallel_z_plane = false;
    
    if(circ_norm == glm::vec3{1.0f, 0.0f, 0.0f} || circ_norm == glm::vec3{-1.0f, 0.0f, 0.0f}) {
        parallel_x_plane = true;
    } else if(circ_norm == glm::vec3{0.0f, 1.0f, 0.0f} || circ_norm == glm::vec3{0.0f, -1.0f, 0.0f}) {
        parallel_y_plane = true;
    } else if(circ_norm == glm::vec3{0.0f, 0.0f, 1.0f} || circ_norm == glm::vec3{0.0f, 0.0f, -1.0f}) {
        parallel_z_plane = true;
    }

    glm::vec3 a;
    if(circ_norm.x != 0) {
        float a1 = (-circ_norm.y - circ_norm.z)/(circ_norm.x);
        a = glm::vec3{a1, 1, 1};
    } else if (circ_norm.y != 0) {
        float a1 = (-circ_norm.x - circ_norm.z)/(circ_norm.y);
        a = glm::vec3{1, a1, 1};
    } else {
        float a1 = (-circ_norm.y - circ_norm.x)/(circ_norm.z);
        a = glm::vec3{1, 1, a1};
    }
    glm::vec3 cross = glm::cross(a, circ_norm);
    a = glm::normalize(a);
    cross = glm::normalize(cross);

    float angle = glm::atan(cross.x / a.x); // -pi/2 --- +pi/2
    float other_angle = angle + glm::radians(180.0f);
    float x_one = radius*(glm::cos(angle)*a.x + glm::sin(angle)*cross.x);
    float x_two = radius*(glm::cos(other_angle)*a.x + glm::sin(other_angle)*cross.x);
    if(x_one > x_two) {
        max_vals[0] = x_two + circ_cent.x;
        max_vals[1] = x_one + circ_cent.x;
    } else {
        max_vals[0] = x_one + circ_cent.x;
        max_vals[1] = x_two + circ_cent.x;
    }

    angle = glm::atan(cross.y / a.y); // -pi/2 --- +pi/2
    other_angle = angle + glm::radians(180.0f);
    float y_one = radius*(glm::cos(angle)*a.y + glm::sin(angle)*cross.y);
    float y_two = radius*(glm::cos(other_angle)*a.y + glm::sin(other_angle)*cross.y);
    if(y_one > y_two) {
        max_vals[2] = y_two + circ_cent.y;
        max_vals[3] = y_one + circ_cent.y;
    } else {
        max_vals[2] = y_one + circ_cent.y;
        max_vals[3] = y_two + circ_cent.y;
    }

    angle = glm::atan(cross.z / a.z); // -pi/2 --- +pi/2
    other_angle = angle + glm::radians(180.0f);
    float z_one = radius*(glm::cos(angle)*a.z + glm::sin(angle)*cross.z);
    float z_two = radius*(glm::cos(other_angle)*a.z + glm::sin(other_angle)*cross.z);
    if(z_one > z_two) {
        max_vals[4] = z_two + circ_cent.z;
        max_vals[5] = z_one + circ_cent.z;
    } else {
        max_vals[4] = z_two + circ_cent.z;
        max_vals[5] = z_one + circ_cent.z;
    }

    if(parallel_x_plane) {
        max_vals[0] = circ_cent.x;
        max_vals[1] = circ_cent.x;
    } else if(parallel_y_plane) {
        max_vals[2] = circ_cent.y;
        max_vals[3] = circ_cent.y;
    } else if(parallel_z_plane) {
        max_vals[4] = circ_cent.z;
        max_vals[5] = circ_cent.z;
    }

    float max_bound = static_cast<float>(VOXEL_GRID_LENGTH);

    max_vals[0] = std::max(0.0f, std::min(max_vals[0], cone_tip.x));
    max_vals[1] = std::min(max_bound, std::max(max_vals[1], cone_tip.x));
    max_vals[2] = std::max(0.0f, std::min(max_vals[2], cone_tip.y));
    max_vals[3] = std::min(max_bound, std::max(max_vals[3], cone_tip.y));
    max_vals[4] = std::max(0.0f, std::min(max_vals[4], cone_tip.z));
    max_vals[5] = std::min(max_bound, std::max(max_vals[5], cone_tip.z));
}


void World::printTrees() {
    tree->printTree();
}


//////////////////////////
// PRIVATE FUNCTIONS BELOW
//////////////////////////

// I could try to batch up the buds and then update the shadow value
void World::updateShade(glm::vec3 bud) {
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

void World::runTimeStep(){
    updateWorldState();
    distributeResources();
    appendNewShoots();
}


void World::updateWorldState() {
    // Reset shadow values
    memset(voxels, 0, VOXEL_CUBIC_AREA);
    //get bud positions and update shading per bud
    std::vector<glm::vec3> budLocations = tree->getTreeBudLocations();
    for(glm::vec3 bud : budLocations) {
        // Could possibly batch up bud locations to do fewer updates
        updateShade(bud);
    }
}


void World::distributeResources() {
    tree->distributeLight(this);
}


void World::appendNewShoots() {
    tree->growNewShoots(this);
}


glm::vec3 World::project_dir_to_circle_plane(glm::vec3 circle_norm, glm::vec3 circle_center, glm::vec3 dir, float  radius) {
    glm::vec3 dir_cross_norm = glm::cross(circle_norm, dir);
    glm::mat4 rot_matrix(1);
    // Might be a problem if the cross product is 0
    rot_matrix = glm::rotate(rot_matrix, glm::radians(90.0f), dir_cross_norm);
    glm::vec3 proj_vect = glm::vec3(rot_matrix * glm::vec4(circle_norm, 1.0f));
    proj_vect = glm::normalize(proj_vect) * radius;

    return (proj_vect + circle_center);
}


float World::round_down_near_half(float num) {
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


float World::round_up_near_half(float num) {
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


bool World::is_point_in_cone(glm::vec3 cone_tip, glm::vec3 tip_to_base, float radius, glm::vec3 point) {
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