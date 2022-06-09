#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <algorithm>

enum DIRECTION {POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z};


void printVec3(glm::vec3 vec) {
    std::cout << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}" << std::endl;
}


glm::vec3 project_dir_to_circle_plane(glm::vec3 circle_norm, glm::vec3 circle_center, glm::vec3 dir, float  radius);
float round_up_near_half(float num);
float round_down_near_half(float num);

void getMaxCoordinates2(glm::vec3 cone_tip, glm::vec3 circ_cent, glm::vec3 circ_norm, float radius, float* max_vals) {
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
    printVec3(a);
    printVec3(cross);


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

    max_vals[0] = std::max(0.0f, std::min(max_vals[0], cone_tip.x));
    max_vals[1] = std::min(201.0f, std::max(max_vals[1], cone_tip.x));
    max_vals[2] = std::max(0.0f, std::min(max_vals[2], cone_tip.y));
    max_vals[3] = std::min(201.0f, std::max(max_vals[3], cone_tip.y));
    max_vals[4] = std::max(0.0f, std::min(max_vals[4], cone_tip.z));
    max_vals[5] = std::min(201.0f, std::max(max_vals[5], cone_tip.z));
}



int main(int argc, char** argv) {
    glm::vec3 cone_tip {100, 100, 0.5};
    glm::vec3 circ_norm {1, 0, 0};
    glm::vec3 circ_center{104, 100, 0.5};
    glm::vec3 search_dir{1, 0, 0};
    float radius = 6.479f;

    // float min_x = getMaxCoordinate(search_dir, circ_center, glm::vec3{-1, 0, 0}, radius);
    // float max_x = getMaxCoordinate(search_dir, circ_center, glm::vec3{1, 0, 0}, radius);
    // float min_y = getMaxCoordinate(search_dir, circ_center, glm::vec3{0, -1, 0}, radius);
    // float max_y = getMaxCoordinate(search_dir, circ_center, glm::vec3{0, 1, 0}, radius);
    // float min_z = getMaxCoordinate(search_dir, circ_center, glm::vec3{0, 0, -1}, radius);
    // float max_z = getMaxCoordinate(search_dir, circ_center, glm::vec3{0, 0, 1}, radius);
    
    float max[6];
    getMaxCoordinates2(cone_tip, circ_center, search_dir, radius, max);

    // std::cout << min_x << std::endl;
    // std::cout << max_x << std::endl;
    // std::cout << min_y << std::endl;
    // std::cout << max_y << std::endl;
    // std::cout << min_z << std::endl;
    // std::cout << max_z << std::endl;

    std::cout << "please print" << std::endl;
    std::cout << max[0] << std::endl;
    std::cout << max[1] << std::endl;
    std::cout << max[2] << std::endl;
    std::cout << max[3] << std::endl;
    std::cout << max[4] << std::endl;
    std::cout << max[5] << std::endl;
    


    return 0;
}

// Only do unit vectors in axis direction
float getMaxCoordinate(glm::vec3 cone_tip, glm::vec3 circle_norm, glm::vec3 circle_center, DIRECTION dir, float radius) {
    glm::vec3 proj_vec;
    if(dir == POS_X) {
        proj_vec = glm::vec3{1, 0, 0};
    } else if(dir == NEG_X) {
        proj_vec = glm::vec3{-1, 0, 0};
    } else if(dir == POS_Y) {
        proj_vec = glm::vec3{0, 1, 0};
    } else if(dir == NEG_Y) {
        proj_vec = glm::vec3{0, -1, 0};
    } else if(dir == POS_Z) {
        proj_vec = glm::vec3{0, 0, 1};
    } else if(dir == NEG_Z) {
        proj_vec = glm::vec3{0, 0, -1};
    }
    
    glm::vec3 projection = project_dir_to_circle_plane(circle_norm, circle_center, proj_vec, radius);

    if(projection == glm::vec3{0.0f, 0.0f, 0.0f}) {
        //circle lies on plane
        
    }

    float min, max;
    if(dir == POS_X) {
        min = 0;
        max = std::max(std::max(projection.x, cone_tip.x), 201.0f);
    } else if(dir == NEG_X) {
        proj_vec = glm::vec3{-1, 0, 0};
    } else if(dir == POS_Y) {
        proj_vec = glm::vec3{0, 1, 0};
    } else if(dir == NEG_Y) {
        proj_vec = glm::vec3{0, -1, 0};
    } else if(dir == POS_Z) {
        proj_vec = glm::vec3{0, 0, 1};
    } else if(dir == NEG_Z) {
        proj_vec = glm::vec3{0, 0, -1};
    }

    float max_val = 3;

    return 0.0f;
}
//    TODO FINISH THIS FUNCTION
   
//     // Generate rectangular prism that encloses cone
//     glm::vec3 pos_z_proj = project_dir_to_circle_plane(circle_norm, circle_center, dir, radius);
//     glm::vec3 neg_z_proj = project_dir_to_circle_plane(circle_norm, circle_center, dir, radius);
//     glm::vec3 pos_y_proj = project_dir_to_circle_plane(circle_norm, circle_center, dir, radius);
//     glm::vec3 neg_y_proj = project_dir_to_circle_plane(circle_norm, circle_center, dir, radius);
//     glm::vec3 pos_x_proj = project_dir_to_circle_plane(circle_norm, circle_center, dir, radius);
//     glm::vec3 neg_x_proj = project_dir_to_circle_plane(circle_norm, circle_center, dir, radius);

//     //TODO CHECK FOR NEGATIVE VALUES YOU DO NOT DO THAT CURRENTLY
//     float max_z = round_down_near_half(std::max(pos_z_proj.z, point.z));
//     float min_z = round_up_near_half(std::min(neg_z_proj.z, point.z));
//     float max_y = round_down_near_half(std::max(pos_y_proj.y, point.y));
//     float min_y = round_up_near_half(std::min(neg_y_proj.y, point.y));
//     float max_x = round_down_near_half(std::max(pos_x_proj.x, point.x));
//     float min_x = round_up_near_half(std::min(neg_x_proj.x, point.x));
// }

glm::vec3 project_dir_to_circle_plane(glm::vec3 circle_norm, glm::vec3 circle_center, glm::vec3 dir, float  radius) {
    glm::vec3 dir_cross_norm = glm::cross(circle_norm, dir);
    
    if(dir_cross_norm == glm::vec3{0.0f, 0.0f, 0.0f}) {
        return glm::vec3{0.0f, 0.0f, 0.0f};
    }

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