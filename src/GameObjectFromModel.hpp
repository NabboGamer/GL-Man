#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObjectBase.hpp"
#include "Model.hpp"

class GameObjectFromModel : public GameObjectBase {

public:
    Model* model;

    GameObjectFromModel(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions,
                        std::vector<float> rotations, std::vector<glm::vec3> scaling,
                        Shader* shader, Model* model);
    ~GameObjectFromModel();

    // Override del metodo Draw
    void Draw() override;

private:
    unsigned int  instanceVBO;

    void initRenderData();

};