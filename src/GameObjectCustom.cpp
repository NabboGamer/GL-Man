#include "GameObjectCustom.hpp"

GameObjectCustom::GameObjectCustom(glm::vec3 position, glm::vec3 direction, float scale, Shader* shader, std::vector<float>& mesh, Texture2D* diffuseTexture, Texture2D* specularTexture)
	            : GameObjectBase(position, direction, scale, shader), mesh(mesh), diffuseTexture(diffuseTexture), specularTexture(specularTexture) {
	this->initRenderData();
}

GameObjectCustom::~GameObjectCustom() {
	//delete texture;
}

void GameObjectCustom::initRenderData() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(float), this->mesh.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    this->vertexCount = this->mesh.size() / 8;
}

void GameObjectCustom::Draw() {
    this->shader->Use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, this->position);
    float angle = glm::atan(this->direction.x, this->direction.z);
    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(this->scale));
    this->shader->SetMatrix4("model", model);

    // Bind diffuse map
    this->shader->SetInteger("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE0);
    this->diffuseTexture->Bind();
    // Bind specular map
    this->shader->SetInteger("material.specular", 1);
    glActiveTexture(GL_TEXTURE1);
    this->specularTexture->Bind();

    this->shader->SetFloat("material.shininess", 1.0f);

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->vertexCount));
    glBindVertexArray(0);
}
