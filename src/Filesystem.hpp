#pragma once

#ifndef ROOT_PATH
#define ROOT_PATH "C:/Users/stolf/dev/Progetto Grafica Tridimensionale Avanzata/GL-Man/src" // Insert here your root path for the project src dir
#endif

#include <string>
#include <cstdlib>

class FileSystem {

private:
    typedef std::string(*Builder)(const std::string& path);

public:
    static std::string getPath(const std::string& path) {
        static std::string(*pathBuilder)(std::string const&) = getPathBuilder();
        return (*pathBuilder)(path);
    }

private:
    static std::string const& getRoot() {
        static char* envRoot = nullptr;
        size_t size = 0;

        // Use _dupenv_s instead of getenv
        _dupenv_s(&envRoot, &size, "ROOT_PATH");

        static std::string root = (envRoot != nullptr ? envRoot : ROOT_PATH);
        if (envRoot != nullptr) free(envRoot); // Free the allocated memory
        return root;
    }

    static Builder getPathBuilder() {
        if (getRoot() != "")
            return &FileSystem::getPathRelativeRoot;
        else
            return &FileSystem::getPathRelativeBinary;
    }

    static std::string getPathRelativeRoot(const std::string& path) {
        return getRoot() + std::string("/") + path;
    }

    static std::string getPathRelativeBinary(const std::string& path) {
        return "../" + path;
    }
};
