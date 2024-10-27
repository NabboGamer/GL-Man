#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObjectBase.hpp"
#include "Texture2D.hpp"

class GameObjectCustom : public GameObjectBase {

public:
    std::vector<float>   mesh;
    Texture2D*           diffuseTexture;
    Texture2D*           specularTexture;

    GameObjectCustom(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions,
                     std::vector<float> rotations, std::vector<glm::vec3> scaling,
                     Shader* shader, std::vector<float>& mesh,
                     Texture2D* diffuseTexture, Texture2D* specularTexture);
    ~GameObjectCustom();

    // Override del metodo Draw
    void Draw() override;

private:
    unsigned int  VAO;
    unsigned int  VBO;
    unsigned int  instanceVBO;
    size_t        vertexCount;

    void initRenderData();

};