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
    glm::vec3   position;
    glm::vec3   direction;
    float       scale
    // render state
    ModelRenderer       renderer
    std::vector<float>  mesh
    Texture2D           texture;
    // constructor
    GameObject(glm::vec3 position, glm::vec3 direction, float scale, ModelRenderer& renderer, std::vector<float>& mesh, Texture2D& texture);
    // draw model
    virtual void Draw();

private:
    size_t modelIndex;

};
