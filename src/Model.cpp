#include <windows.h>

#include "Model.hpp"
#include "LoggerManager.hpp"

Model::Model(string const& path, bool gamma) : gammaCorrection(gamma), minBounds(FLT_MAX), maxBounds(-FLT_MAX) {
    loadModel(path);
}

Model::~Model() {

}

void Model::Draw(Shader& shader, size_t numInstances) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader, numInstances);
    }   
}

void Model::loadModel(string const& path) {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LoggerManager::LogError("ASSIMP {}", importer.GetErrorString());
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh processedMesh = processMesh(mesh, scene);

        // Aggiornamento globale della bounding box con i vertici della mesh
        for (const auto& vertex : processedMesh.vertices) {
            this->minBounds.x = std::min(this->minBounds.x, vertex.Position.x);
            this->minBounds.y = std::min(this->minBounds.y, vertex.Position.y);
            this->minBounds.z = std::min(this->minBounds.z, vertex.Position.z);

            this->maxBounds.x = std::max(this->maxBounds.x, vertex.Position.x);
            this->maxBounds.y = std::max(this->maxBounds.y, vertex.Position.y);
            this->maxBounds.z = std::max(this->maxBounds.z, vertex.Position.z);
        }

        meshes.push_back(processedMesh);
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        // we declare a placeholder vector since assimp uses its own 
        // vector class that doesn't directly convert to glm's vec3 
        // class so we transfer the data to this placeholder glm::vec3 first.
        glm::vec3 vector; 
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    aiString materialName;
    aiReturn ret;
    ret = material->Get(AI_MATKEY_NAME, materialName);
    
    LoggerManager::LogDebug("Mesh Material: {}", materialName.C_Str());
    aiColor3D color;
    glm::vec3 vecColor;
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color))
        vecColor = glm::vec3(color.r, color.g, color.b);
        LoggerManager::LogDebug("Ka=({},{},{})", vecColor.x ,vecColor.y ,vecColor.z);
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color))
        vecColor = glm::vec3(color.r, color.g, color.b);
        LoggerManager::LogDebug("Kd=({},{},{})", vecColor.x, vecColor.y, vecColor.z);
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color))
        vecColor = glm::vec3(color.r, color.g, color.b);
        LoggerManager::LogDebug("Ks=({},{},{})", vecColor.x, vecColor.y, vecColor.z);

    glm::vec3 ambientColor(1.0f), diffuseColor(1.0f), specularColor(1.0f);
    bool hasAmbientTexture = false;
    bool hasDiffuseTexture = false;
    bool hasSpecularTexture = false;

    if (material->GetTextureCount(aiTextureType_AMBIENT) > 0) {
        // 1. ambient maps
        std::vector<Texture> ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient");
        textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
        hasAmbientTexture = true;
    }
    else {
        aiColor3D color;
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color)) {
            ambientColor = glm::vec3(color.r, color.g, color.b);
        }
        else {
            ambientColor = glm::vec3(0.5);
        }
    }

    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        // 2. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        hasDiffuseTexture = true;
    }
    else {
        aiColor3D color;
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
            diffuseColor = glm::vec3(color.r, color.g, color.b);
        }
        else {
            diffuseColor = glm::vec3(0.5);
        }
            
    }

    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        // 3. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        hasSpecularTexture = true;
    }
    else {
        aiColor3D color;
        if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color)) {
            specularColor = glm::vec3(color.r, color.g, color.b);
        }
        else {
            specularColor = glm::vec3(0.5);
        }
            
    }

    LoggerManager::LogDebug("hasAmbientTexture={}",(hasAmbientTexture==true)?"true":"false");
    LoggerManager::LogDebug("hasDiffuseTexture={}",(hasDiffuseTexture ==true)?"true":"false");
    LoggerManager::LogDebug("hasSpecularTexture={}",(hasSpecularTexture ==true)?"true":"false");
    LoggerManager::LogDebug("");

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures, ambientColor, diffuseColor, specularColor, hasAmbientTexture, hasDiffuseTexture, hasSpecularTexture);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip) {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char* path, const string& directory, bool gamma) {
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        LoggerManager::LogError("MODEL: Texture failed to load at path: {}", path);
        stbi_image_free(data);
    }

    return textureID;
}

std::pair<glm::vec3, glm::vec3> Model::GetBoundingBox() const {
    return { this->minBounds, this->maxBounds };
}