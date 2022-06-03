#include <iostream>
#include "world.hpp"

int main(int argc, char** argv) {
    float a = 0.25;
    float b = 1.5;
    float apical = 0.5;
    float determinacy = 0.5;
    float branching_angle = 45;
    float res_distr = 2;
    float max_vigor = 50;
    float shedding = 1;

    World* new_world = new World(
        a, 
        b, 
        apical, 
        determinacy, 
        branching_angle, 
        res_distr, 
        max_vigor, 
        shedding);

    new_world->printTrees();

    delete new_world;

    return 0;
}