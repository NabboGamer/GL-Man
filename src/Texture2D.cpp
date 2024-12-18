#include "LoggerManager.hpp"
#include "Texture2D.hpp"


Texture2D::Texture2D()
         : id(0), width(0), height(0), internalFormat(GL_RGB), imageFormat(GL_RGB),
           wrapS(GL_REPEAT), wrapT(GL_REPEAT), filterMin(GL_LINEAR_MIPMAP_LINEAR), filterMax(GL_LINEAR) {
}

void Texture2D::Generate(unsigned int width, unsigned int height, unsigned char* data, GLenum dataType) {
    if (this->id == 0) {
        glGenTextures(1, &this->id); // Generate ID only if needed
    }
    // Set width and height
    this->width = width;
    this->height = height;

    // Fallback formats in case `internalFormat` and `imageFormat` are invalid
    GLenum internalFormat = this->internalFormat != 0 ? this->internalFormat : GL_RGB;
    GLenum imageFormat = this->imageFormat != 0 ? this->imageFormat : GL_RGB;

    // Check texture ID and generate if not exists
    if (this->id == 0) {
        glGenTextures(1, &this->id);
    }

    // Create the texture
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imageFormat, dataType, data);
    
    // Generate mipmap after loading main texture
    glGenerateMipmap(GL_TEXTURE_2D);

    // Wrapping and filtering settings, including mipmapping mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapS != 0 ? this->wrapS : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapT != 0 ? this->wrapT : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filterMin != 0 ? this->filterMin : GL_LINEAR_MIPMAP_LINEAR); // Enable linear mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filterMax != 0 ? this->filterMax : GL_LINEAR); // Mipmapping does not apply to the MAG filter

    // Check for OpenGL errors when setting parameters
#ifdef _DEBUG
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LoggerManager::LogError("TEXTURE2D: {}", error);
    }
#endif

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind() const {
    glBindTexture(GL_TEXTURE_2D, this->id);
}
