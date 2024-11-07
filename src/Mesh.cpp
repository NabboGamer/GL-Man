#include "Mesh.hpp"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures,
           glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, bool hasAmbientTexture,
           bool hasDiffuseTexture, bool hasSpecularTexture)
    : vertices(vertices), indices(indices), textures(textures), ambientColor(ambient), 
      diffuseColor(diffuse), specularColor(specular), hasAmbientTexture(hasAmbientTexture),
      hasDiffuseTexture(hasDiffuseTexture), hasSpecularTexture(hasSpecularTexture) {

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    setupMesh();
}

Mesh::~Mesh() {
}

void Mesh::setupMesh() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader, size_t numInstances) {
    // bind appropriate textures
    unsigned int ambientNr = 1;
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        string number;
        string name = textures[i].type;
        if (name == "texture_ambient")
            number = std::to_string(ambientNr++);
        else if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++); // transfer unsigned int to stream

        // now set the sampler to the correct texture unit
        glUniform1i(glGetUniformLocation(shader.id, (name + number).c_str()), i);
        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    shader.SetBool("hasAmbientTexture", this->hasAmbientTexture);
    shader.SetBool("hasDiffuseTexture", this->hasDiffuseTexture);
    shader.SetBool("hasSpecularTexture", this->hasSpecularTexture);
    shader.SetVector3f("ambientColor", this->ambientColor);
    shader.SetVector3f("diffuseColor", this->diffuseColor);
    shader.SetVector3f("specularColor", this->specularColor);

    // draw mesh
    glBindVertexArray(VAO);
    if (numInstances > 1) {
        GLsizei indicesSize = static_cast<GLsizei>(this->indices.size());
        GLsizei numInstancesCasted = static_cast<GLsizei>(numInstances);
        glDrawElementsInstanced(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0, numInstancesCasted);
    } else {
        GLsizei indicesSize = static_cast<GLsizei>(this->indices.size());
        glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}