#include <windows.h>

#include "GameObjectCustom.hpp"
#include "Utility.hpp"

GameObjectCustom::GameObjectCustom(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions,
                                   std::vector<float> rotations, std::vector<glm::vec3> scaling,
                                   Shader* shader, std::vector<float>& mesh, 
                                   Texture2D* diffuseTexture, Texture2D* specularTexture)
	            : GameObjectBase(positions, directions, rotations, scaling, shader), 
                  mesh(mesh), diffuseTexture(diffuseTexture), specularTexture(specularTexture), 
                  minBounds(FLT_MAX), maxBounds(-FLT_MAX) {

    this->calculateBoundingBox();

    // Calculate the original bounding box
    auto [minBounds, maxBounds] = this->GetBoundingBox();

    // Calculate the offset to center pmin at the origin
    this->centerOffset = -minBounds;

    this->initRenderData();
}

GameObjectCustom::~GameObjectCustom() {
	//delete texture;
}

void GameObjectCustom::initRenderData() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->instanceVBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(float), this->mesh.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Instanced Buffer for model matrices
    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // Attributes for the 4x4 model array (split into 4 vec4 attributes)
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(3 + i, 1); // Change per instance, not per vertex
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    this->vertexCount = this->mesh.size() / 8;
}

void GameObjectCustom::Draw() {
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

    // Bind diffuse map
    this->shader->SetInteger("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE0);
    this->diffuseTexture->Bind();
    // Bind specular map
    this->shader->SetInteger("material.specular", 1);
    glActiveTexture(GL_TEXTURE1);
    this->specularTexture->Bind();

    //this->shader->SetFloat("material.shininess", 1.0f);

    glBindVertexArray(this->VAO);
    if (this->numInstances > 1) {
        GLsizei vertexCount = static_cast<GLsizei>(this->vertexCount);
        GLsizei numInstances = static_cast<GLsizei>(this->numInstances);
        glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, numInstances);
    } else {
        GLsizei vertexCount = static_cast<GLsizei>(this->vertexCount);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
    glBindVertexArray(0);
}

void GameObjectCustom::calculateBoundingBox() {

    // Itera sui vertici, saltando 8 valori alla volta per passare da un vertice all'altro
    for (size_t i = 0; i < this->mesh.size(); i += 8) {
        glm::vec3 vertex(this->mesh[i], this->mesh[i + 1], this->mesh[i + 2]);

        // Aggiorna il minimo e massimo per ogni coordinata
        this->minBounds.x = std::min(this->minBounds.x, vertex.x);
        this->minBounds.y = std::min(this->minBounds.y, vertex.y);
        this->minBounds.z = std::min(this->minBounds.z, vertex.z);

        this->maxBounds.x = std::max(this->maxBounds.x, vertex.x);
        this->maxBounds.y = std::max(this->maxBounds.y, vertex.y);
        this->maxBounds.z = std::max(this->maxBounds.z, vertex.z);
    }

}

std::pair<glm::vec3, glm::vec3> GameObjectCustom::GetBoundingBox() const {
    return { this->minBounds, this->maxBounds };
}

std::pair<glm::vec3, glm::vec3> GameObjectCustom::GetTransformedBoundingBox(size_t instanceIndex) const {
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
