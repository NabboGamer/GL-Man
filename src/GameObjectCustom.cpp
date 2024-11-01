#include "GameObjectCustom.hpp"

GameObjectCustom::GameObjectCustom(std::vector<glm::vec3> positions, std::vector<glm::vec3> directions,
                                   std::vector<float> rotations, std::vector<glm::vec3> scaling,
                                   Shader* shader, std::vector<float>& mesh, 
                                   Texture2D* diffuseTexture, Texture2D* specularTexture)
	            : GameObjectBase(positions, directions, rotations, scaling, shader), 
                  mesh(mesh), diffuseTexture(diffuseTexture), specularTexture(specularTexture) {
	this->initRenderData();
    this->calculatePmin();
    this->calculatePmax();
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

        // Translation to exactly position the pmin vertex at the origin
        model = glm::translate(model, -this->pMin);

        model = glm::translate(model, this->positions[i]);

        float angle = glm::atan(this->directions[i].x, this->directions[i].z);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::rotate(model, this->rotations[i], glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::scale(model, this->scaling[i]);

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
        glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(this->vertexCount), this->numInstances);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->vertexCount));
    }
    glBindVertexArray(0);
}

void GameObjectCustom::calculatePmin() {
    glm::vec3 exactPmin(this->mesh[0], this->mesh[1], this->mesh[2]);

    for (size_t i = 0; i < this->mesh.size(); i += 8) { // Each vertex has 8 values ​​(xyz + normals + uv)
        float x = this->mesh[i];
        float y = this->mesh[i + 1];
        float z = this->mesh[i + 2];

        // If we find a vertex with a smaller y value, we select it as the new pmin
        if (y < exactPmin.y ||
            (y == exactPmin.y && x < exactPmin.x) ||
            (y == exactPmin.y && x == exactPmin.x && z < exactPmin.z)) {
            exactPmin = glm::vec3(x, y, z);
        }
    }

    this->pMin = exactPmin;
}

void GameObjectCustom::calculatePmax() {
    glm::vec3 exactPmax(this->mesh[0], this->mesh[1], this->mesh[2]);

    for (size_t i = 0; i < this->mesh.size(); i += 8) { // Each vertex has 8 values ​​(xyz + normals + uv)
        float x = this->mesh[i];
        float y = this->mesh[i + 1];
        float z = this->mesh[i + 2];

        // If we find a vertex with a higher y value, we select it as the new pmax
        if (y > exactPmax.y ||
            (y == exactPmax.y && x > exactPmax.x) ||
            (y == exactPmax.y && x == exactPmax.x && z > exactPmax.z)) {
            exactPmax = glm::vec3(x, y, z);
        }
    }

    this->pMax = exactPmax;
}