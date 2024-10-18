#include <iostream>
#include "ModelRenderer.hpp"

ModelRenderer::ModelRenderer() {}

ModelRenderer::~ModelRenderer() {
    for (const auto& model : models) {
        glDeleteVertexArrays(1, &model.VAO);
        glDeleteBuffers(1, &model.VBO);
    }
}

size_t ModelRenderer::InitModel(Shader* shader, const std::vector<float>& mesh) {
    ModelData modelData;
    modelData.shader = shader;
    glGenVertexArrays(1, &modelData.VAO);
    glGenBuffers(1, &modelData.VBO);

    glBindVertexArray(modelData.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, modelData.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(float), mesh.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    modelData.vertexCount = mesh.size() / 5;
    models.push_back(modelData);

    return models.size() - 1;  // Return index model
}

void ModelRenderer::DrawModel(size_t modelIndex, const glm::vec3& position, const glm::vec3& direction, float scale, Texture2D* texture) {
    if (modelIndex >= models.size()) {
        std::cout << "ModelRenderer::ERROR Model not initialized" << std::endl;
        return;
    }

    ModelData modelData = models[modelIndex];
    modelData.shader->Use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    float angle = glm::atan(direction.x, direction.z);
    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale));
    modelData.shader->SetMatrix4("model", model);

    modelData.shader->SetInteger("texture_diffuse1", 0, false);
    glActiveTexture(GL_TEXTURE0);
    texture->Bind();

    glBindVertexArray(modelData.VAO);
    glDrawArrays(GL_TRIANGLES, 0, modelData.vertexCount);
    glBindVertexArray(0);
}