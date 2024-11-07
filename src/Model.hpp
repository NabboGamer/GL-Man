#pragma once

#include <string>
#include <fstream>
#include <sstream>
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
	// The need for a no-arg constructor comes from the fact that the ResourceManager class uses std::map to store Model instances. 
	// For std::map to create or copy objects internally, the type of the stored objects (in this case, Model) must be constructible and assignable.
	Model() { }
	Model(string const& path, bool gamma = false);
	~Model();

	// draws the model, and thus all its meshes
	void Draw(Shader& shader, size_t numInstances);

	std::pair<glm::vec3, glm::vec3> GetBoundingBox() const;

private:
	glm::vec3        minBounds; // Minimum coordinates of the bounding box
	glm::vec3        maxBounds; // Maximum coordinates of the bounding box

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
