#include <cstdlib>

#include "GameObjectBase.hpp"
#include "LoggerManager.hpp"


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
        LoggerManager::LogFatal("The sizes of the initialization vectors passed to the constructor are inconsistent!");
        exit(1);
    }
}
