#include <windows.h>

#include "GameObjectFromModel.hpp"

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

        model = glm::rotate(model, glm::radians(this->rotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));

        float angle = glm::atan(this->directions[i].x, this->directions[i].z);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::scale(model, this->scaling[i]);

        // Apply offset to center pmin at origin
        model = glm::translate(model, this->centerOffset);

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

float roundToSixDecimals1(float value) {
    float roundedValue = std::round(value * 1e6) / 1e6;
    // If the value is close to zero (positive or negative), let's approximate it to zero
    return (std::abs(roundedValue) < 1e-6f) ? 0.0f : roundedValue;
}

// Apply rounding to six decimal places on each component of glm::vec3
glm::vec3 approximateToSixDecimals1(const glm::vec3& vec) {
    return glm::vec3(
        roundToSixDecimals1(vec.x),
        roundToSixDecimals1(vec.y),
        roundToSixDecimals1(vec.z)
    );
}

std::pair<glm::vec3, glm::vec3> GameObjectFromModel::GetTransformedBoundingBox(size_t instanceIndex) const {
    auto [minBounds, maxBounds] = this->GetBoundingBox();

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Applying Instance Transformation
    modelMatrix = glm::translate(modelMatrix, this->positions[instanceIndex]);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(this->rotations[instanceIndex]), glm::vec3(0.0f, 1.0f, 0.0f));
    float angle = glm::atan(this->directions[instanceIndex].x, this->directions[instanceIndex].z);
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, this->scaling[instanceIndex]);
    // Apply centering offset
    modelMatrix = glm::translate(modelMatrix, this->centerOffset);

    glm::vec3 transformedMin = glm::vec3(modelMatrix * glm::vec4(minBounds, 1.0f));
    glm::vec3 transformedMax = glm::vec3(modelMatrix * glm::vec4(maxBounds, 1.0f));

    glm::vec3 finalMin = glm::min(transformedMin, transformedMax);
    glm::vec3 finalMax = glm::max(transformedMin, transformedMax);

    return { approximateToSixDecimals1(finalMin), approximateToSixDecimals1(finalMax) };
}
