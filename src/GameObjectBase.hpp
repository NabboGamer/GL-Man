#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.hpp"


// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.
class GameObjectBase {

public:
    // object state
    glm::vec3   position;
    glm::vec3   direction;
    float       scale;
    
    // render state
    Shader* shader;

    // constructor/destructor
    GameObjectBase(glm::vec3 position, glm::vec3 direction, float scale, Shader* shader);
    virtual ~GameObjectBase();

    // Pure virtual method that forces subclasses to implement their own Draw method
    virtual void Draw() = 0;

};
