#include <cstdlib>
#include <windows.h>

#include "GameObjectBase.hpp"
#include "LoggerManager.hpp"


GameObjectBase::GameObjectBase(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions, 
                               std::vector<float> rotations, std::vector<glm::vec3> scaling, Shader* shader)
              : positions(positions), directions(directions), rotations(rotations), scaling(scaling), shader(shader) {
    this->validityCheck();
}

GameObjectBase::~GameObjectBase() {
    //delete shader;
}

size_t GameObjectBase::GetNumInstances() const {
    return this->numInstances;
}

void GameObjectBase::SetNumInstances(const size_t newNumInstances) {
    this->numInstances = newNumInstances;
}

void GameObjectBase::UpdateNumInstance() {
    this->validityCheck();
    const size_t actualNumInstance = this->positions.size();
    this->SetNumInstances(actualNumInstance);
}

void GameObjectBase::validityCheck() {
    if (this->positions.size()  == this->directions.size() &&
        this->directions.size() == this->rotations.size() &&
        this->rotations.size()  == this->scaling.size()) {
        this->numInstances = this->positions.size();
    } else {
        LoggerManager::LogFatal("The sizes of the vectors are inconsistent!");
        exit(-1);
    }
}


