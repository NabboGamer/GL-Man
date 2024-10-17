#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Texture2D.hpp"
#include "ModelRenderer.hpp"


// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.
class GameObject {

public:
    // object state
    glm::vec2   position, direction;
    // render state
    Texture2D   texture;
    // constructor(s)
    GameObject();
    GameObject(glm::vec2 pos, glm::vec2 direction, Texture2D texture);
    // draw sprite
    virtual void Draw(ModelRenderer &renderer);

};
