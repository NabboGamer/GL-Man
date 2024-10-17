#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture2D.hpp"
#include "Shader.hpp"


class ModelRenderer {

public:
    // Constructor: inits shaders, VAO, VBO and mesh.
    // Mesh is a array of float that contain position, normal and texture coordinate per vertex of a mesh.
    ModelRenderer(Shader &shader, unsigned int VAO, unsigned int VBO, float* mesh);
    // Destructor
    ~ModelRenderer();
    // Renders a defined model
    void DrawModel(Texture2D &texture, glm::vec2 position);

private:
    // Render state
    Shader       shader; 
    unsigned int VAO,VBO;
    float*       mesh;
    // Initializes and configures the quad's buffer and vertex attributes
    void initRenderData();

};
