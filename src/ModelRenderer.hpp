#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture2D.hpp"
#include "Shader.hpp"

// Object that is responsible for rendering game objects.
// Each game object must expose its state to this object
// in order to ask it to be rendered.
class ModelRenderer {

public:
    ModelRenderer();
    ~ModelRenderer();

    // Returns the index of the initialized model
    size_t InitModel(Shader* shader, const std::vector<float>& mesh);

    void DrawModel(size_t modelIndex, const glm::vec3& position, const glm::vec3& direction, float scale, Texture2D* texture);

private:
    struct ModelData {
        Shader* shader;
        unsigned int VAO;
        unsigned int VBO;
        size_t vertexCount;
    };
    std::vector<ModelData> models;

};