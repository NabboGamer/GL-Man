#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>

#include "Mesh.hpp"
#include "Shader.hpp"

using namespace std;

class Model {

public:
  // model data 
  vector<Texture>  textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once
  vector<Mesh>     meshes;
  string           directory;
  bool             gammaCorrection;

  // constructor, expects a filepath to a 3D model.
  Model(string const& path, bool gamma = false);
  ~Model();

  // draws the model, and thus all its meshes
  void Draw(Shader& shader, size_t numInstances);

private:
  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(string const& path);

  // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
  void processNode(aiNode* node, const aiScene* scene);

  Mesh processMesh(aiMesh* mesh, const aiScene* scene);

  // checks all material textures of a given type and loads the textures if they're not loaded yet.
  // the required info is returned as a Texture struct.
  vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

  unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

};
