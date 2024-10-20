#include "GameObjectBase.hpp"


GameObjectBase::GameObjectBase(glm::vec3 position, glm::vec3 direction, float scale, Shader* shader)
              : position(position), direction(direction), scale(scale), shader(shader) { }

GameObjectBase::~GameObjectBase() {
    //delete shader;
}