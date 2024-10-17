#include "ModelRenderer.hpp"


ModelRenderer::ModelRenderer(Shader& shader, unsigned int VAO, unsigned int VBO, float* mesh) {
    this->shader = shader;
    this->VAO = VAO;
    this->VBO = VBO;
    this->mesh = mesh;
    this->initRenderData();
}

ModelRenderer::~ModelRenderer() {
    // Delete the VAO
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;  // Set to 0 to avoid accidental double deletion
    }

    // Delete the VBO
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;  // Set to 0 to avoid accidental double deletion
    }
}


void ModelRenderer::DrawModel(Texture2D &texture, glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 color) {
    this->shader.Use();
    //glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(position, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

    //model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); // move origin of rotation to center of quad
    //model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f)); // then rotate
    //model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); // move origin back

    //model = glm::scale(model, glm::vec3(size, 1.0f)); // last scale

    //this->shader.SetMatrix4("model", model);

    //// render textured quad
    //this->shader.SetVector3f("spriteColor", color);


    this->shader.SetInteger("texture_diffuse1", 0, false);
    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void ModelRenderer::initRenderData() {
    // Configure VAO and VBO
    // Generate VAO and VBO
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    // Bind the VAO and the VBO
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    // Load the data in the VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->mesh), this->mesh, GL_STATIC_DRAW);

    // Define how the data for a particular vertex attribute is organized in the buffer loaded(the VBO) 
    // (location 0 and location 2) and enable the attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind the VAO and the VBO (optional)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}