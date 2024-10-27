#include <iostream>
#include <cstdlib>

#include "GameObjectBase.hpp"


GameObjectBase::GameObjectBase(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions, 
                               std::vector<float> rotations, std::vector<glm::vec3> scaling, Shader* shader)
              : positions(positions), directions(directions), rotations(rotations), scaling(scaling), shader(shader) {
    validityCheck();
}

GameObjectBase::~GameObjectBase() {
    //delete shader;
}

void GameObjectBase::validityCheck() {
    if (this->positions.size() == this->directions.size() &&
        this->directions.size() == this->rotations.size() &&
        this->rotations.size() == this->scaling.size()) {
        this->numInstances = this->positions.size();
    } else {
        // Set text color to red
        std::cerr << "\033[31m";
        std::cerr << "ERROR::GameObjectBase:: The sizes of the initialization vectors passed to the constructor are inconsistent!" << std::endl;
        // Reset text color to default
        std::cerr << "\033[0m";
        exit(1);
    }
}
