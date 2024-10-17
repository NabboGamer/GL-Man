#pragma once

#include <string>
#include <cstdlib>


#ifndef ROOT_PATH
#define ROOT_PATH "C:/Users/stolf/dev/Progetto Grafica Tridimensionale Avanzata/GL-Man" // Insert here your root path for the project
#endif

class FileSystem {

private:
    typedef std::string(*Builder) (const std::string& path);

public:
    static std::string getPath(const std::string& path) {
        static std::string(*pathBuilder)(std::string const&) = getPathBuilder();
        return (*pathBuilder)(path);
    }

private:
    static std::string const& getRoot() {
        static char const* envRoot = getenv("ROOT_PATH");
        static char const* givenRoot = (envRoot != nullptr ? envRoot : ROOT_PATH);
        static std::string root = (givenRoot != nullptr ? givenRoot : "");
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
        return "../../../" + path;
    }
};
