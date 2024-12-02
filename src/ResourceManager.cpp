#include <sstream>
#include <fstream>
#include <windows.h>

#include "ResourceManager.hpp"
#include "Filesystem.hpp"
#include "LoggerManager.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Instantiate static variables
std::map<std::string, Texture2D>    ResourceManager::Textures;
std::map<std::string, Shader>       ResourceManager::Shaders;
std::map<std::string, Model>        ResourceManager::Models;


Shader ResourceManager::LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name) {
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader& ResourceManager::GetShader(std::string name) {
    return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const char *file, std::string name, const bool useSRGB) {
    Textures[name] = loadTextureFromFile(file, useSRGB);
    return Textures[name];
}

Texture2D& ResourceManager::GetTexture(std::string name) {
    return Textures[name];
}

Model ResourceManager::LoadModel(const std::string& path, std::string name, const bool useSRGB) {
    Models[name] = Model(FileSystem::getPath(path), useSRGB);
    return Models[name];
}

Model& ResourceManager::GetModel(std::string name) {
    return Models[name];
}

void ResourceManager::Clear() {
    // (properly) delete all shaders	
    for (auto iter : Shaders)
        glDeleteProgram(iter.second.id);
    // (properly) delete all textures
    for (auto iter : Textures)
        glDeleteTextures(1, &iter.second.id);
}

Shader ResourceManager::loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile) {
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try {
        // open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr) {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::exception e) {
        LoggerManager::LogError("RESOURCEMANAGER: Failed to read shader files");
    }
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();
    const char *gShaderCode = geometryCode.c_str();
    // 2. now create shader object from source code
    Shader shader;
    shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, const bool useSRGB) {
    // Create the texture object
    Texture2D texture;

    int width, height, nrChannels;
    unsigned char* data = nullptr;
    GLenum dataType = GL_UNSIGNED_BYTE;  // Let's start with the 8-bit data type

    // Attempting to load as 8-bit image
    data = stbi_load(file, &width, &height, &nrChannels, 0);
    if (!data) {  // If that fails, try the 16-bit data type.
        unsigned short* data16 = stbi_load_16(file, &width, &height, &nrChannels, 0);
        if (data16) {
            dataType = GL_UNSIGNED_SHORT;
            data = reinterpret_cast<unsigned char*>(data16);
        }
        else {  // If even 16-bit fails, try the float data type.
            float* dataFloat = stbi_loadf(file, &width, &height, &nrChannels, 0);
            if (dataFloat) {
                dataType = GL_FLOAT;
                data = reinterpret_cast<unsigned char*>(dataFloat);
            }
            else {
                // If all loading fail, report the error and exit
                LoggerManager::LogError("RESOURCEMANAGER: Failed to load image file: {}", file);
                return texture;
            }
        }
    }


    // Set formats based on `nrChannels` and `dataType`
    if (nrChannels == 1) { // Grayscale images are not affected by sRGB
        if (dataType == GL_UNSIGNED_BYTE) {
            texture.internalFormat = GL_R8;
        }
        else if (dataType == GL_UNSIGNED_SHORT) {
            texture.internalFormat = GL_R16;
        }
        else if (dataType == GL_FLOAT) {
            texture.internalFormat = GL_R32F;
        }
        texture.imageFormat = GL_RED;
    }
    else if (nrChannels == 3) {
        if (dataType == GL_UNSIGNED_BYTE) {
            texture.internalFormat = useSRGB ? GL_SRGB8 : GL_RGB8;
        }
        else if (dataType == GL_UNSIGNED_SHORT) {
            texture.internalFormat = GL_RGB16;
        }
        else if (dataType == GL_FLOAT) {
            texture.internalFormat = GL_RGB32F;
        }
        texture.imageFormat = GL_RGB;
    }
    else if (nrChannels == 4) {
        if (dataType == GL_UNSIGNED_BYTE) {
            texture.internalFormat = useSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        }
        else if (dataType == GL_UNSIGNED_SHORT) {
            texture.internalFormat = GL_RGBA16;
        }
        else if (dataType == GL_FLOAT) {
            texture.internalFormat = GL_RGBA32F;
        }
        texture.imageFormat = GL_RGBA;
    }
    else {
        LoggerManager::LogError("RESOURCEMANAGER: Unsupported image format: {} channels", nrChannels);
        stbi_image_free(data);
        return texture;
    }

    // Genera la texture con il tipo di dato corretto
    texture.Generate(width, height, data, dataType);

    // Libera la memoria dell'immagine
    stbi_image_free(data);

    return texture;
}