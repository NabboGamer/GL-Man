#include <fstream>
#include <sstream>

#include "GameLevel.hpp"
#include "ResourceManager.hpp"
#include "GameObjectCustom.hpp"
#include "GameObjectFromModel.hpp"

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

const float L = 1.0f;  // Depth along  x
const float W = 0.1f;  // Height along y
const float H = 1.0f;  // Length along z

std::vector<float> parallelepiped_mesh = {
    // Front face (z = +H/2)
    -L / 2, -W / 2,  H / 2,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  // bottom-left
     L / 2, -W / 2,  H / 2,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,  // bottom-right
     L / 2,  W / 2,  H / 2,   0.0f,  0.0f,  1.0f,   1.0f,  1.0f,  // top-right
     L / 2,  W / 2,  H / 2,   0.0f,  0.0f,  1.0f,   1.0f,  1.0f,  // top-right
    -L / 2,  W / 2,  H / 2,   0.0f,  0.0f,  1.0f,   0.0f,  1.0f,  // top-left
    -L / 2, -W / 2,  H / 2,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  // bottom-left

    // Back face (z = -H/2)
    -L / 2, -W / 2, -H / 2,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f,  // bottom-left
     L / 2, -W / 2, -H / 2,   0.0f,  0.0f, -1.0f,   1.0f,  0.0f,  // bottom-right
     L / 2,  W / 2, -H / 2,   0.0f,  0.0f, -1.0f,   1.0f,  1.0f,  // top-right
     L / 2,  W / 2, -H / 2,   0.0f,  0.0f, -1.0f,   1.0f,  1.0f,  // top-right
    -L / 2,  W / 2, -H / 2,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  // top-left
    -L / 2, -W / 2, -H / 2,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f,  // bottom-left

    // Left face (x = -L/2)
    -L / 2,  W / 2,  H / 2,  -1.0f,  0.0f,  0.0f,   1.0f,  1.0f,  // top-right
    -L / 2,  W / 2, -H / 2,  -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  // top-left
    -L / 2, -W / 2, -H / 2,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  // bottom-left
    -L / 2, -W / 2, -H / 2,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  // bottom-left
    -L / 2, -W / 2,  H / 2,  -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  // bottom-right
    -L / 2,  W / 2,  H / 2,  -1.0f,  0.0f,  0.0f,   1.0f,  1.0f,  // top-right

    // Right face (x = +L/2)
     L / 2,  W / 2,  H / 2,   1.0f,  0.0f,  0.0f,   1.0f,  1.0f,  // top-right
     L / 2,  W / 2, -H / 2,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  // top-left
     L / 2, -W / 2, -H / 2,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  // bottom-left
     L / 2, -W / 2, -H / 2,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  // bottom-left
     L / 2, -W / 2,  H / 2,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  // bottom-right
     L / 2,  W / 2,  H / 2,   1.0f,  0.0f,  0.0f,   1.0f,  1.0f,  // top-right

     // Bottom face (y = -W/2)
     -L / 2, -W / 2, -H / 2,   0.0f, -1.0f,  0.0f,   0.0f,  1.0f,  // top-left
      L / 2, -W / 2, -H / 2,   0.0f, -1.0f,  0.0f,   1.0f,  1.0f,  // top-right
      L / 2, -W / 2,  H / 2,   0.0f, -1.0f,  0.0f,   1.0f,  0.0f,  // bottom-right
      L / 2, -W / 2,  H / 2,   0.0f, -1.0f,  0.0f,   1.0f,  0.0f,  // bottom-right
     -L / 2, -W / 2,  H / 2,   0.0f, -1.0f,  0.0f,   0.0f,  0.0f,  // bottom-left
     -L / 2, -W / 2, -H / 2,   0.0f, -1.0f,  0.0f,   0.0f,  1.0f,  // top-left

     // Top face (y = +W/2)
     -L / 2,  W / 2, -H / 2,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  // top-left
      L / 2,  W / 2, -H / 2,   0.0f,  1.0f,  0.0f,   1.0f,  1.0f,  // top-right
      L / 2,  W / 2,  H / 2,    0.0f,  1.0f,  0.0f,  1.0f,  0.0f,  // bottom-right
      L / 2,  W / 2,  H / 2,    0.0f,  1.0f,  0.0f,  1.0f,  0.0f,  // bottom-right
     -L / 2,  W / 2,  H / 2,    0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  // bottom-left
     -L / 2,  W / 2, -H / 2,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f   // top-left
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
    this->mazeFloor->Draw();
    this->mazeWall->Draw();
    this->dot->Draw();
    this->energizer->Draw();
}

//bool GameLevel::IsCompleted() {
//    for (GameObject &tile : this->Bricks)
//        if (!tile.IsSolid && !tile.Destroyed)
//            return false;
//    return true;
//}

void GameLevel::init(std::vector<std::vector<unsigned int>> wallData) {
    // calculate dimensions
    unsigned int height = static_cast<unsigned int>(wallData.size());
    unsigned int width = static_cast<unsigned int>(wallData[0].size()); // note we can index vector at [0] since this function is only called if height > 0
    
    // initialize level wall based on wallData		
    for (unsigned int x = 0; x < height; x++) {
        for (unsigned int z = 0; z < width; z++) {
            // check block type from level data (2D level array)
            // is a wall? is a dot? or is a hallway?
            if (wallData[x][z] == 1) {
                // Wall
                glm::vec3 position = glm::vec3(static_cast<float>(height - (x + 1)), 0.0f, static_cast<float>(z));
                this->mazeWallPositions.push_back(position);
            }
            else if (wallData[x][z] == 2) {
                // Floor
                glm::vec3 position = glm::vec3(static_cast<float>(height - (x + 1)), -0.1f, static_cast<float>(z));
                this->mazeFloorPositions.push_back(position);
                // Dot
                if (x + 1 < height && z + 1 < width) { // Checking the limits to avoid exceeding the array
                    if (wallData[x][z + 1] == 2 && wallData[x + 1][z] == 2 && wallData[x + 1][z + 1] == 2) {
                        glm::vec3 position = glm::vec3(static_cast<float>(height - (x + 1)) - 0.5f, 0.25f, static_cast<float>(z) + 0.75f);
                        this->dotPositions.push_back(position);
                    }
                }
            } else if (wallData[x][z] == 3) {
                // Floor
                glm::vec3 position = glm::vec3(static_cast<float>(height - (x + 1)), -0.1f, static_cast<float>(z));
                this->mazeFloorPositions.push_back(position);
                // Energizer
                if (x + 1 < height && z + 1 < width) { // Checking the limits to avoid exceeding the array
                    if (wallData[x][z + 1] == 3 && wallData[x + 1][z] == 3 && wallData[x + 1][z + 1] == 3) {
                        glm::vec3 position = glm::vec3(static_cast<float>(height - (x + 1)) - 0.25f, 0.0f, static_cast<float>(z) + 0.5f);
                        this->energizerPositions.push_back(position);
                    }
                }
            } else if (wallData[x][z] == 0) {
                // Floor
                glm::vec3 position = glm::vec3(static_cast<float>(height - (x + 1)), -0.1f, static_cast<float>(z));
                this->mazeFloorPositions.push_back(position);
            }
        }
    }
    size_t numInstancesMazeWall = this->mazeWallPositions.size();
    std::vector<glm::vec3> mazeWallDirections(numInstancesMazeWall, glm::vec3(0.0f, 0.0f, 1.0f));
    std::vector<float>     mazeWallRotations(numInstancesMazeWall, 0.0f);
    std::vector<glm::vec3> mazeWallScaling(numInstancesMazeWall, glm::vec3(1.0f));
    this->mazeWall = new GameObjectCustom(this->mazeWallPositions,
                                          mazeWallDirections,
                                          mazeWallRotations,
                                          mazeWallScaling,
                                          &ResourceManager::GetShader("mazeWallShader"),
                                          cube_mesh,
                                          &ResourceManager::GetTexture("mazeWallDiffuseTexture"),
                                          &ResourceManager::GetTexture("mazeWallSpecularTexture"));

    size_t numInstancesMazeFloor = this->mazeFloorPositions.size();
    std::vector<glm::vec3> mazeFloorDirections(numInstancesMazeFloor, glm::vec3(0.0f, 0.0f, 1.0f));
    std::vector<float>     mazeFloorRotations(numInstancesMazeFloor, 0.0f);
    std::vector<glm::vec3> mazeFloorScaling(numInstancesMazeFloor, glm::vec3(1.0f));
    this->mazeFloor = new GameObjectCustom(this->mazeFloorPositions,
                                           mazeFloorDirections,
                                           mazeFloorRotations,
                                           mazeFloorScaling,
                                           &ResourceManager::GetShader("mazeFloorShader"),
                                           parallelepiped_mesh,
                                           &ResourceManager::GetTexture("mazeFloorDiffuseTexture"),
                                           &ResourceManager::GetTexture("mazeFloorSpecularTexture"));

    size_t numInstancesDot = this->dotPositions.size();
    std::vector<glm::vec3> dotDirections(numInstancesDot, glm::vec3(0.0f, 0.0f, 1.0f));
    std::vector<float>     dotRotations(numInstancesDot, 0.0f);
    std::vector<glm::vec3> dotScaling(numInstancesDot, glm::vec3(0.25f));
    this->dot = new GameObjectFromModel(this->dotPositions,
                                        dotDirections,
                                        dotRotations,
                                        dotScaling,
                                        &ResourceManager::GetShader("dotShader"),
                                        &ResourceManager::GetModel("dotModel"));

    size_t numInstancesEnergizer = this->energizerPositions.size();
    std::vector<glm::vec3> energizerDirections(numInstancesEnergizer, glm::vec3(0.0f, 0.0f, 1.0f));
    std::vector<float>     energizerRotations(numInstancesEnergizer, 0.0f);
    std::vector<glm::vec3> energizerScaling(numInstancesEnergizer, glm::vec3(0.5f));
    this->energizer = new GameObjectFromModel(this->energizerPositions,
                                              energizerDirections,
                                              energizerRotations,
                                              energizerScaling,
                                              &ResourceManager::GetShader("energizerShader"),
                                              &ResourceManager::GetModel("energizerModel"));
}