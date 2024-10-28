#include <fstream>
#include <sstream>

#include "GameLevel.hpp"
#include "ResourceManager.hpp"
#include "GameObjectCustom.hpp"

std::vector<float> cube_mesh = {
    // Front face (z = +0.5)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  // bottom-left
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  // bottom-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  // top-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  // top-right
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,  // top-left
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  // bottom-left

    // Back face (z = -0.5)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  // bottom-left
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,  // bottom-right
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,  // top-right
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,  // top-right
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,  // top-left
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  // bottom-left

    // Left face (x = -0.5)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // top-right
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // top-left
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // bottom-right
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  // top-right

    // Right face (x = +0.5)
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // top-right
    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // top-left
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // bottom-right
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  // top-right

    // Bottom face (y = -0.5)
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  // top-left
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,  // top-right
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,  // bottom-right
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,  // bottom-right
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  // bottom-left
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  // top-left

    // Top face (y = +0.5)
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  // top-left
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  1.0f,  // top-right
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  0.0f,  // bottom-right
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  0.0f,  // bottom-right
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  0.0f,  // bottom-left
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f   // top-left
};


GameLevel::~GameLevel() {
    //delete mazeWall;
}

void GameLevel::Load(const char *file) {
    // clear old data
    this->mazeWallPositions.clear();
    // load from file
    unsigned int wallCode;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> wallData;
    if (fstream) {
        // read each line from level file
        while (std::getline(fstream, line)) {
            std::istringstream sstream(line);
            std::vector<unsigned int> row;

            // read each word separated by spaces
            // Please Note: the >> operator automatically ignores 
            // white spaces (spaces, tabs, new lines) between numbers.
            while (sstream >> wallCode) {
                row.push_back(wallCode);
            }

            wallData.push_back(row);
        }
        if (wallData.size() > 0) {
            this->init(wallData);
        }
    }
}

void GameLevel::Draw() {
    this->mazeWall->Draw();
}

//bool GameLevel::IsCompleted() {
//    for (GameObject &tile : this->Bricks)
//        if (!tile.IsSolid && !tile.Destroyed)
//            return false;
//    return true;
//}

void GameLevel::init(std::vector<std::vector<unsigned int>> wallData) {
    // calculate dimensions
    unsigned int height = wallData.size();
    unsigned int width = wallData[0].size(); // note we can index vector at [0] since this function is only called if height > 0
    
    // initialize level wall based on wallData		
    for (unsigned int x = 0; x < height; x++) {
        for (unsigned int z = 0; z < width; z++) {
            // check block type from level data (2D level array)
            // is a wall? is a dot? or is a hallway?
            if (wallData[x][z] == 1) {
                // 0.5f is the preliminary movement which serves to bring the pmin of the cube to (0,0,0)
                glm::vec3 position = glm::vec3(0.5f + static_cast<float>(height - (x+1)), 0.5f, 0.5f + static_cast<float>(z));
                this->mazeWallPositions.push_back(position);
            } else if (wallData[x][z] == 2) {

            } else if (wallData[x][z] == 0) {

            }
        }
    }
    size_t numInstances = this->mazeWallPositions.size();
    std::vector<glm::vec3> mazeWallDirections(numInstances, glm::vec3(1.0f, 0.0f, 0.0f));
    std::vector<float>     mazeWallRotations(numInstances, 0.0f);
    std::vector<glm::vec3> mazeWallScaling(numInstances, glm::vec3(1.0f));
    this->mazeWall = new GameObjectCustom(mazeWallPositions,
                                          mazeWallDirections,
                                          mazeWallRotations,
                                          mazeWallScaling,
                                          &ResourceManager::GetShader("mazeWallShader"),
                                          cube_mesh,
                                          &ResourceManager::GetTexture("mazeWallDiffuseTexture"),
                                          &ResourceManager::GetTexture("mazeWallSpecularTexture"));
}