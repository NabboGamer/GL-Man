#pragma once

#ifndef ROOT_PATH
#define ROOT_PATH "" // Insert here your root path for the project src dir
#endif

#include <string>
#include <cstdlib>
#include <windows.h>

#include "LoggerManager.hpp"

class FileSystem {

private:
    typedef std::string(*Builder)(const std::string& path);

public:
    static std::string getPath(const std::string& path) {
        static std::string(*pathBuilder)(std::string const&) = getPathBuilder();
        return normalizePath((*pathBuilder)(path));
    }

private:

    static Builder getPathBuilder() {
        return &FileSystem::getPathRelativeRoot;
    }

    static std::string getPathRelativeRoot(const std::string& path) {
        return getRoot() + std::string("/") + path;
    }

    static std::string const& getRoot() {
        static std::string root;
        if (!root.empty()) return root;

        // First try using the ROOT_PATH environment variable
        char* envRoot = nullptr;
        size_t size = 0;
        _dupenv_s(&envRoot, &size, "ROOT_PATH");
        if (envRoot != nullptr && std::string(envRoot).empty() == false) {
            root = envRoot;
            free(envRoot);
        }
        else {
            // If the variable is not defined, use the path to the executable
            root = getExecutablePath() + std::string("/..");
        }
        return root;
    }

    static std::string getExecutablePath() {
        char buffer[MAX_PATH];
        const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        if (length == 0 || length == MAX_PATH) {
            LoggerManager::LogFatal("FILESYSTEM: Could not get the path to the executable!");
            exit(-1);
        }
        const std::string fullPath(buffer);
        return fullPath.substr(0, fullPath.find_last_of("\\/"));
    }

    // Function to normalize paths (backslash -> slash)
    static std::string normalizePath(const std::string& path) {
        std::string normalized = path;
        for (auto& ch : normalized) {
            if (ch == '\\') {
                ch = '/';
            }
        }
        return normalized;
    }
};
