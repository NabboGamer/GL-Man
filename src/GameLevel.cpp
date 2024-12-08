#include <windows.h>
#include <fstream>
#include <sstream>

#include "GameLevel.hpp"
#include "ResourceManager.hpp"
#include "GameObjectCustom.hpp"
#include "GameObjectFromModel.hpp"
#include "LoggerManager.hpp"

static std::vector<float> cube_mesh = {
    
    // Front face (z = +0.5)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  // bottom-left
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  // bottom-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  // top-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  // top-right
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,  // top-left
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  // bottom-left

    // Back face (z = -0.5)
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  // bottom-left
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,  // bottom-right
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,  // top-right
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,  // top-right
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,  // top-left
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  // bottom-left

    // Left face (x = -0.5)
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // top-right
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // top-left
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  // top-right

    // Right face (x = +0.5)
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // top-right
    0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // top-left
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom-left
    0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // bottom-right
    0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  // top-right

    // Bottom face (y = -0.5)
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  // top-left
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,  // top-right
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,  // bottom-right
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,  // bottom-right
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  // bottom-left
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  // top-left

    // Top face (y = +0.5)
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,  // top-left
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  1.0f,  // top-right
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  0.0f,  // bottom-right
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,   0.0f,  1.0f,  0.0f,  // bottom-right
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  0.0f,  // bottom-left
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f   // top-left
    };

constexpr float L = 1.0f;  // Depth along  x
constexpr float W = 0.1f;  // Height along y
constexpr float H = 1.0f;  // Length along z

GameLevel::~GameLevel() {
    /*delete mazeWall;
    delete mazeFloor;
    delete dot;
    delete energizer;*/
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
            this->levelMatrixDim.first = wallData.size();
            this->levelMatrixDim.second = wallData[0].size();
            this->init(wallData);
        }
    }
}

void GameLevel::Draw(const double deltaTime) {
    if (this->mazeFloor->GetNumInstances() > 0) {
        this->mazeFloor->Draw();
    }

    if (this->shouldSpawnBonusSymbol(deltaTime)) {
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        this->bonusSymbol->Draw();
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        //glDisable(GL_DEPTH_TEST);
        this->bonusSymbolStencil->Draw();
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        //glEnable(GL_DEPTH_TEST);
    }

    if (this->mazeWall->GetNumInstances() > 0) {
        this->mazeWall->Draw();
    }
    if (this->dot->GetNumInstances() > 0) {
        this->dot->Draw();
    }
    if (this->energizer->GetNumInstances() > 0) {
        this->visibilityTimer += deltaTime;

        if (this->energizerVisible && this->visibilityTimer >= VISIBLE_DURATION) {
            // Switch to invisible state
            this->energizerVisible = false;
            this->visibilityTimer = 0.0f; // Reset the timer
        }
        else if (!this->energizerVisible && this->visibilityTimer >= INVISIBLE_DURATION) {
            // Switch to visible state
            this->energizerVisible = true;
            this->visibilityTimer = 0.0f; // Reset the timer
        }

        // Draw the energizer only if visible
        if (this->energizerVisible) {
            // Disable normal drawing of energizer in favor of the version with bloom
            //this->energizer->Draw();
            this->energizerLight->Draw();
        }
    }

    if (this->playerTakeBonusSymbol) {
        this->fruitCounter->Draw();
    }
    if (this->lifeCounter->GetNumInstances() > 0) {
        this->lifeCounter->Draw();
    }
}

bool GameLevel::GetPlayerTakeBonusSymbol() const {
	return this->playerTakeBonusSymbol;
}

void GameLevel::SetPlayerTakeBonusSymbol(const bool value) {
    this->playerTakeBonusSymbol = value;
}

double GameLevel::GetFirstActivationTimeAccumulator() const {
	return this->firstActivationTimeAccumulator;
}

void GameLevel::SetFirstActivationTimeAccumulator(const double value) {
    this->firstActivationTimeAccumulator = value;
}

double GameLevel::GetSecondActivationTimeAccumulator() const {
	return this->secondActivationTimeAccumulator;
}

void GameLevel::SetSecondActivationTimeAccumulator(const double value) {
    this->secondActivationTimeAccumulator = value;
}

int GameLevel::GetSymbolActive() const {
	return this->symbolActive;
}

void GameLevel::SetSymbolActive(const int value) {
    this->symbolActive = value;
}

void GameLevel::SetBonusSymbolPosition(const glm::vec3 newBonusSymbolPosition) const {
    this->bonusSymbol->positions[0] = newBonusSymbolPosition;
    this->bonusSymbolStencil->positions[0] = newBonusSymbolPosition;
}

bool GameLevel::IsCompleted() const {
    if (this->dot->GetNumInstances() == 0 &&
        this->energizer->GetNumInstances() == 0) {
        return true;
    }
    return false;
}

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
                auto position = glm::vec3(static_cast<float>(height - (x + 1)), 0.0f, static_cast<float>(z));
                this->mazeWallPositions.push_back(position);
            }
            else if (wallData[x][z] == 2) {
                // Floor
                auto position = glm::vec3(static_cast<float>(height - (x + 1)), -0.1f, static_cast<float>(z));
                this->mazeFloorPositions.push_back(position);
                // Dot
                if (x + 1 < height && z + 1 < width) { // Checking the limits to avoid exceeding the array
                    if (wallData[x][z + 1] == 2 && wallData[x + 1][z] == 2 && wallData[x + 1][z + 1] == 2) {
                        auto position = glm::vec3(static_cast<float>(height - (x + 1)) - 0.5f, 0.25f, static_cast<float>(z) + 0.75f);
                        this->dotPositions.push_back(position);
                    }
                }
            } else if (wallData[x][z] == 3) {
                // Floor
                auto position = glm::vec3(static_cast<float>(height - (x + 1)), -0.1f, static_cast<float>(z));
                this->mazeFloorPositions.push_back(position);
                // Energizer
                if (x + 1 < height && z + 1 < width) { // Checking the limits to avoid exceeding the array
                    if (wallData[x][z + 1] == 3 && wallData[x + 1][z] == 3 && wallData[x + 1][z + 1] == 3) {
                        auto position = glm::vec3(static_cast<float>(height - (x + 1)) - 0.25f, 0.0f, static_cast<float>(z) + 0.5f);
                        this->energizerPositions.push_back(position);
                    }
                }
            } else if (wallData[x][z] == 0) {
                // Floor
                auto position = glm::vec3(static_cast<float>(height - (x + 1)), -0.1f, static_cast<float>(z));
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
    std::vector<glm::vec3> mazeFloorScaling(numInstancesMazeFloor, glm::vec3(L, W, H));
    this->mazeFloor = new GameObjectCustom(this->mazeFloorPositions,
                                           mazeFloorDirections,
                                           mazeFloorRotations,
                                           mazeFloorScaling,
                                           &ResourceManager::GetShader("mazeFloorShader"),
                                           cube_mesh,
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

    this->energizerLight = new GameObjectFromModel(this->energizerPositions,
                                                   energizerDirections,
                                                   energizerRotations,
                                                   energizerScaling,
                                                   &ResourceManager::GetShader("energizerLightShader"),
                                                   &ResourceManager::GetModel("energizerLightModel"));
    
    size_t numInstancesBonusSymbol = 1;
	std::vector<glm::vec3> bonusSymbolPositions(numInstancesBonusSymbol, glm::vec3(13.0f, 0.0f, 14.0f));
	std::vector<glm::vec3> bonusSymbolDirections(numInstancesBonusSymbol, glm::vec3(0.0f, 0.0f, 1.0f));
    std::vector<float>     bonusSymbolRotations(numInstancesBonusSymbol, 90.0f);
    std::vector<glm::vec3> bonusSymbolScaling(numInstancesBonusSymbol, glm::vec3(1.0f));
    this->bonusSymbol = new GameObjectFromModel(bonusSymbolPositions,
                                                bonusSymbolDirections,
                                                bonusSymbolRotations,
                                                bonusSymbolScaling,
                                                &ResourceManager::GetShader("bonusSymbolShader"),
                                                &ResourceManager::GetModel("cherriesModel"));

    size_t numInstancesBonusSymbolStencil = 1;
    std::vector<glm::vec3> bonusSymbolPositionsStencil(numInstancesBonusSymbolStencil, glm::vec3(13.0f, 0.0f, 14.0f));
    std::vector<glm::vec3> bonusSymbolDirectionsStencil(numInstancesBonusSymbolStencil, glm::vec3(0.0f, 0.0f, 1.0f));
    std::vector<float>     bonusSymbolRotationsStencil(numInstancesBonusSymbolStencil, 90.0f);
    std::vector<glm::vec3> bonusSymbolScalingStencil(numInstancesBonusSymbolStencil, glm::vec3(1.05f));
    this->bonusSymbolStencil = new GameObjectFromModel(bonusSymbolPositionsStencil,
                                                       bonusSymbolDirectionsStencil,
                                                       bonusSymbolRotationsStencil,
                                                       bonusSymbolScalingStencil,
                                                       &ResourceManager::GetShader("stencilShader"),
                                                       &ResourceManager::GetModel("cherriesStencilModel"));

    size_t numInstancesFruitCounter = 1;
    std::vector<glm::vec3> fruitCounterPositions(numInstancesFruitCounter, glm::vec3(-2.5f, 0.0f, 26.5f));
    std::vector<glm::vec3> fruitCounterDirections(numInstancesFruitCounter, glm::vec3(0.0f, 0.0f, 1.0f));
    std::vector<float>     fruitCounterRotations(numInstancesFruitCounter, 90.0f);
    std::vector<glm::vec3> fruitCounterScaling(numInstancesFruitCounter, glm::vec3(1.5f));
    this->fruitCounter = new GameObjectFromModel(fruitCounterPositions,
                                                 fruitCounterDirections,
                                                 fruitCounterRotations,
                                                 fruitCounterScaling,
                                                 &ResourceManager::GetShader("bonusSymbolShader"),
                                                 &ResourceManager::GetModel("cherriesFruitCounterModel"));


    std::vector<glm::vec3> lifeCounterPositions  = { glm::vec3(-2.0f, 0.0f, 0.5f), glm::vec3(-2.0f, 0.0f, 2.5f), glm::vec3(-2.0f, 0.0f, 4.5f) };
    std::vector<glm::vec3> lifeCounterDirections = { glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3( 0.0f, 0.0f,-1.0f) };
    std::vector<float>     lifeCounterRotations  = { 0.0f, 0.0f, 0.0f };
    std::vector<glm::vec3> lifeCounterScaling    = { glm::vec3(0.245f), glm::vec3(0.25f) , glm::vec3(0.25f) };
    this->lifeCounter = new GameObjectFromModel(lifeCounterPositions,
												lifeCounterDirections,
											    lifeCounterRotations,
												lifeCounterScaling,
                                                &ResourceManager::GetShader("lifeCounterShader"),
                                                &ResourceManager::GetModel("lifeCounterPacmanModel"));

}

bool GameLevel::shouldSpawnBonusSymbol(const double deltaTime) {
    const size_t numInstancesDot = this->dot->positions.size();
    if (numInstancesDot > 62 && numInstancesDot <= 162) {
        this->firstActivationTimeAccumulator += deltaTime;
        if (this->firstActivationTimeAccumulator <= FIRST_ACTIVATION_TIME_LIMIT) {
            this->SetBonusSymbolPosition(glm::vec3(13.0f, 0.0f, 14.0f));
            if (this->symbolActive == 0) this->symbolActive = 1;
            return true;
        } else {
            this->SetBonusSymbolPosition(glm::vec3(-2.0f, 0.0f, -2.0f));
        }
       
    } else if (numInstancesDot <= 62) {
        this->secondActivationTimeAccumulator += deltaTime;
        if (this->secondActivationTimeAccumulator <= SECOND_ACTIVATION_TIME_LIMIT) {
            this->SetBonusSymbolPosition(glm::vec3(13.0f, 0.0f, 14.0f));
            if (this->symbolActive == 1) this->symbolActive = 2;
            return true;
        } else {
            this->SetBonusSymbolPosition(glm::vec3(-2.0f, 0.0f, -2.0f));
        }
    }
	return false;
}
