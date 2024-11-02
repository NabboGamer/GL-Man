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

    // Override of the Draw method
    void Draw() override;

    // Override of the GetBoundingBox method
    std::pair<glm::vec3, glm::vec3> GetBoundingBox() const override;

private:
    unsigned int  instanceVBO;

    void initRenderData();

};