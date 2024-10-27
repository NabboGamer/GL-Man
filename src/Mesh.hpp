#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"

using namespace std;

struct Vertex {
  // position
  glm::vec3 Position;
  // normal
  glm::vec3 Normal;
  // texCoords
  glm::vec2 TexCoords;
  // tangent
  glm::vec3 Tangent;
  // bitangent
  glm::vec3 Bitangent;
};

struct Texture {
  unsigned int id;
  string type;
  string path;
};

class Mesh {

public:
  // mesh Data
  vector<Vertex>       vertices;
  vector<unsigned int> indices;
  vector<Texture>      textures;
  unsigned int		   VAO;

  // constructor
  Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
  ~Mesh();
   
  // render the mesh
  void Draw(Shader& shader);

private:
  // render data 
  unsigned int VBO, EBO, instanceVBO;
  // initializes all the buffer objects/arrays
  void setupMesh();

};