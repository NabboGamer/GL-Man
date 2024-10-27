#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.hpp"


// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.
class GameObjectBase {

public:
    // object state
    std::vector<glm::vec3>   positions;
    std::vector<glm::vec3>   directions;
    std::vector<float>       rotations;
    std::vector<glm::vec3>   scaling;
    
    // render state
    Shader* shader;

    // constructor/destructor
    GameObjectBase(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions, 
                   std::vector<float> rotations, std::vector<glm::vec3> scaling,
                   Shader* shader);
    virtual ~GameObjectBase();

    // Pure virtual method that forces subclasses to implement their own Draw method
    virtual void Draw() = 0;

protected:
    size_t numInstance;

private:
    void validityCheck();

};
