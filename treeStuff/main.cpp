#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include "world.hpp"

int main(int argc, char** argv) {
    float a = 0.15;
    float b = 1.0;
    float apical = 0.7;
    float determinacy = 0.5;
    float branching_angle = 45;
    float res_distr = 2;
    float max_vigor = 50;
    float shedding = 0.5f;

    World* new_world = new World(
        a, 
        b, 
        apical, 
        determinacy, 
        branching_angle, 
        res_distr, 
        max_vigor, 
        shedding);

    for(int i =0; i < 100; i++) {
        new_world->runTimeStep();
        new_world->exportPoints();
        pid_t pid = fork();
        if(pid == 0) {
            //child process
            execl("./skeletongraph", "./skeletongraph", NULL);
        }
        std::cin.get();
        kill(pid, SIGTERM);
    }

    delete new_world;

    return 0;
}