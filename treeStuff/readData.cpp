#include <glm/glm.hpp>

#include <fstream>
#include <iostream>

void printVec3(glm::vec3 vec) {
    std::cout << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
}

int main(int argc, char** argv) {
    std::fstream vfile, ifile;
    vfile.open("tsp.txt", std::fstream::in);
    ifile.open("tsi.txt", std::fstream::in);

    float coord[3];
    size_t index;
    do {
        vfile.read((char*)coord, 3*sizeof(float));

        glm::vec3 point{coord[0], coord[1], coord[2]};
        printVec3(point);
        std::cout << std::endl;
    } while(!vfile.eof() | !vfile.fail());
    if(vfile.eof()) {
        std::cout << "vfile eof" << std::endl;
    }
    if(vfile.fail()) {
        std::cout << "vfile fail" << std::endl;
    }


    do {
        ifile.read((char*)&index, sizeof(size_t));
        std::cout << "Index: " << index << std::endl;
        if(index == 0xFFFFFFFF) {
            std::cout << "RESTART STRIP" << std::endl;
        }
    } while(!ifile.eof() | !ifile.fail());
    if(ifile.eof()) {
        std::cout << "ifile eof" << std::endl;
    }
    if(ifile.fail()) {
        std::cout << "ifile fail" << std::endl;
    }
}