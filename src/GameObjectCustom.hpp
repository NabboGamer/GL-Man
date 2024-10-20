#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObjectBase.hpp"
#include "Texture2D.hpp"

class GameObjectCustom : public GameObjectBase {

public:
    std::vector<float>   mesh;
    Texture2D*           texture;

    GameObjectCustom(glm::vec3 position, glm::vec3 direction, float scale, Shader* shader, std::vector<float>& mesh, Texture2D* texture);
	~GameObjectCustom();

    // Override del metodo Draw
    void Draw() override;

private:
    unsigned int VAO;
    unsigned int VBO;
    size_t vertexCount;

    void initRenderData();

};