#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture2D.hpp"
#include "Shader.hpp"

class ModelRenderer {

public:
    ModelRenderer(Shader& shader);
    ~ModelRenderer();

    // Returns the index of the initialized model
    size_t InitModel(const std::vector<float>& mesh);

    void DrawModel(size_t modelIndex, const glm::mat4 projection, const glm::mat4 view, const glm::vec3& position, const glm::vec3& direction, float scale, Texture2D& texture);

private:
    Shader shader;
    struct ModelData {
        unsigned int VAO;
        unsigned int VBO;
        size_t vertexCount;
    };
    std::vector<ModelData> models;

};