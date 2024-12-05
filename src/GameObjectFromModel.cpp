#include <windows.h>

#include "GameObjectFromModel.hpp"
#include "Utility.hpp"

GameObjectFromModel::GameObjectFromModel(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions,
                                         std::vector<float> rotations, std::vector<glm::vec3> scaling,
                                         Shader* shader, Model* model)
	               : GameObjectBase(positions, directions, rotations, scaling, shader), model(model) {

    // Calculate the original bounding box
    auto [minBounds, maxBounds] = this->GetBoundingBox();

    // Calculate the offset to center pmin at the origin
    this->centerOffset = -minBounds;

	this->initRenderData();
}

GameObjectFromModel::~GameObjectFromModel() {
	//delete texture;
}

void GameObjectFromModel::initRenderData() {
    for (unsigned int i = 0; i < this->model->meshes.size(); i++) {
        unsigned int VAO = this->model->meshes[i].VAO;
        glBindVertexArray(VAO);
        glGenBuffers(1, &this->instanceVBO);

        // Instanced Buffer for model matrices
        glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        // Attributes for the 4x4 model array (split into 4 vec4 attributes)
        for (unsigned int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(5 + i);
            glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(5 + i, 1); // Change per instance, not per vertex
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void GameObjectFromModel::Draw() {
    this->shader->Use();

    std::vector<glm::mat4> modelMatrices(this->numInstances, glm::mat4(1.0f));
    for (size_t i = 0; i < this->numInstances; i++) {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, this->positions[i]);

        model = glm::scale(model, this->scaling[i]);

        // Apply offset to center pmin at origin
        model = glm::translate(model, this->centerOffset);

        model = glm::rotate(model, glm::radians(this->rotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));

        float angle = glm::atan(this->directions[i].x, this->directions[i].z);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        modelMatrices[i] = model;
    }

    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    this->model->Draw(*this->shader, this->numInstances);
}

std::pair<glm::vec3, glm::vec3> GameObjectFromModel::GetBoundingBox() const {
    return this->model->GetBoundingBox();
}

std::pair<glm::vec3, glm::vec3> GameObjectFromModel::GetTransformedBoundingBox(size_t instanceIndex) const {
    auto [minBounds, maxBounds] = this->GetBoundingBox();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, this->positions[instanceIndex]);
    model = glm::scale(model, this->scaling[instanceIndex]);
    model = glm::translate(model, this->centerOffset);
    model = glm::rotate(model, glm::radians(this->rotations[instanceIndex]), glm::vec3(0.0f, 1.0f, 0.0f));
    float angle = glm::atan(this->directions[instanceIndex].x, this->directions[instanceIndex].z);
    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 transformedMin = glm::vec3(model * glm::vec4(minBounds, 1.0f));
    glm::vec3 transformedMax = glm::vec3(model * glm::vec4(maxBounds, 1.0f));

    glm::vec3 finalMin = glm::min(transformedMin, transformedMax);
    glm::vec3 finalMax = glm::max(transformedMin, transformedMax);

    return { Utility::ApproximateVec3ToSixDecimals(finalMin), Utility::ApproximateVec3ToSixDecimals(finalMax) };
}
