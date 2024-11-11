#include <windows.h>
#include <algorithm>
#include <sstream>

//#include <irrklang/irrKlang.h>
//using namespace irrklang;

#include "Game.hpp"
#include "FileSystem.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"
#include "GameObjectBase.hpp"
#include "GameObjectCustom.hpp"
#include "GameObjectFromModel.hpp"
#include "PacMan.hpp"
//#include "particle_generator.h"
//#include "post_processor.h"
//#include "text_renderer.h"


// Game-related State data
PacMan* pacman;
GameObjectBase* ghost;

//ParticleGenerator *Particles;
//PostProcessor     *Effects;
//ISoundEngine      *SoundEngine = createIrrKlangDevice();
//TextRenderer      *Text;

// Initial speed of the player
float PLAYER_SPEED = 7.5f;

Game::Game(unsigned int width, unsigned int height) : state(GAME_ACTIVE), keys(), keysProcessed(), width(width), height(height) { }

Game::~Game() {
    delete pacman;
    /*delete Ball;
    delete Particles;
    delete Effects;
    delete Text;
    SoundEngine->drop();*/
}

void Game::Init() {
    /// Load Shaders
    ResourceManager::LoadShader("./shaders/mazeWall.vs",  "./shaders/mazeWall.fs",  nullptr, "mazeWallShader");
    ResourceManager::LoadShader("./shaders/mazeFloor.vs", "./shaders/mazeFloor.fs", nullptr, "mazeFloorShader");
    ResourceManager::LoadShader("./shaders/dot.vs",       "./shaders/dot.fs",       nullptr, "dotShader");
    ResourceManager::LoadShader("./shaders/dot.vs",       "./shaders/dot.fs",       nullptr, "energizerShader");
    ResourceManager::LoadShader("./shaders/pacman.vs",    "./shaders/pacman.fs",    nullptr, "pacmanShader");
    ResourceManager::LoadShader("./shaders/ghost.vs",     "./shaders/ghost.fs",     nullptr, "ghostShader");
    /*ResourceManager::LoadShader("particle.vs", "particle.fs", nullptr, "particle");
    ResourceManager::LoadShader("post_processing.vs", "post_processing.fs", nullptr, "postprocessing");*/

    /// Configure Shaders
    // Insert uniform variable in vertex shader(only global variables, i.e. the same for all shaders)
    cameraPos = glm::vec3( -17.0, 22.5, 15.0);
    cameraAt  = glm::vec3(  10.0,  1.0, 15.0);
    up        = glm::vec3(   0.0,  1.0,  0.0);
    cameraDir = glm::normalize(cameraPos - cameraAt);
    cameraSide = glm::normalize(glm::cross(up, cameraDir));
    cameraUp = glm::normalize(glm::cross(cameraDir, cameraSide));
    glm::mat4 view = glm::lookAt(cameraPos, cameraAt, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(35.0f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 55.0f);
    ResourceManager::GetShader("mazeWallShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("mazeWallShader").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("mazeFloorShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("mazeFloorShader").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("dotShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("dotShader").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("energizerShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("energizerShader").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("pacmanShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("pacmanShader").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("ghostShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("ghostShader").Use().SetMatrix4("projection", projection);
    // Insert uniform variable in fragment shader(only global variables, i.e. the same for all shaders)
    ResourceManager::GetShader("mazeWallShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("mazeWallShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("mazeWallShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.4f, 0.4f, 0.4f));
    ResourceManager::GetShader("mazeWallShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    ResourceManager::GetShader("mazeWallShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("mazeWallShader").Use().SetFloat("material.shininess", 8.0f);
    ResourceManager::GetShader("mazeFloorShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("mazeFloorShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("mazeFloorShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.4f, 0.4f, 0.4f));
    ResourceManager::GetShader("mazeFloorShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    ResourceManager::GetShader("mazeFloorShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("mazeFloorShader").Use().SetFloat("material.shininess", 8.0f);
    ResourceManager::GetShader("dotShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("dotShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("dotShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.3f, 0.3f, 0.3f));
    ResourceManager::GetShader("dotShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.7f, 0.7f, 0.7f));
    ResourceManager::GetShader("dotShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("dotShader").Use().SetFloat("material.shininess", 32.0f);
    ResourceManager::GetShader("energizerShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("energizerShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("energizerShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.3f, 0.3f, 0.3f));
    ResourceManager::GetShader("energizerShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.7f, 0.7f, 0.7f));
    ResourceManager::GetShader("energizerShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("energizerShader").Use().SetFloat("material.shininess", 32.0f);
    ResourceManager::GetShader("pacmanShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("pacmanShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("pacmanShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.7f, 0.7f, 0.7f));
    ResourceManager::GetShader("pacmanShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
    ResourceManager::GetShader("pacmanShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("pacmanShader").Use().SetFloat("material.shininess", 32.0f);
    ResourceManager::GetShader("ghostShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("ghostShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("ghostShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.7f, 0.7f, 0.7f));
    ResourceManager::GetShader("ghostShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
    ResourceManager::GetShader("ghostShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("ghostShader").Use().SetFloat("material.shininess", 32.0f);

    /// Load Textures
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/wall_diffuse_360.png").c_str(), "mazeWallDiffuseTexture");
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/wall_specular_360.png").c_str(), "mazeWallSpecularTexture");
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/floor_diffuse_360.png").c_str(), "mazeFloorDiffuseTexture");
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/floor_specular_360.png").c_str(), "mazeFloorSpecularTexture");

    /// Load Models
    ResourceManager::LoadModel("../res/objects/powerup/coin/coin.obj", "dotModel");
    ResourceManager::LoadModel("../res/objects/powerup/coin/coin.obj", "energizerModel");
    ResourceManager::LoadModel("../res/objects/ghosts/vulnerable_ghost_blue/vulnerable_ghost_blue1/vulnerable_ghost_blue1.obj", "ghostModel");

    /// Load Levels
    GameLevel levelOne;
    levelOne.Load(FileSystem::getPath("../res/levels/one.lvl").c_str());
    this->Levels.push_back(levelOne);
    this->level = 0;

    /// Configure Game Objects
    pacman = new PacMan();

    std::vector<glm::vec3> modelPositions = { glm::vec3(7.5f, 0.0f, 10.0f) };
    std::vector<glm::vec3> modelDirections = { glm::vec3(0.0f, 0.0f, -1.0f) };
    std::vector<float>     modelRotations = { 90.0f };
    std::vector<glm::vec3> modelScaling = { glm::vec3(1.0f) };

     ghost = new GameObjectFromModel(modelPositions,
                                     modelDirections,
                                     modelRotations,
                                     modelScaling,
                                     &ResourceManager::GetShader("ghostShader"),
                                     &ResourceManager::GetModel("ghostModel"));
    // audio
    //SoundEngine->play2D(FileSystem::getPath("resources/audio/breakout.mp3").c_str(), true);
}

///TODO: Introdurre classe per i fantasmi in modo da gestire sia quelli statici per differenziarli sulla strategia di movimento 
///      sia quelli dinamici in cui oltre che la strategia c'è il meccanismo di animazione come per pacman.

void Game::Update(double dt) {
//    // update objects
//    Ball->Move(dt, this->Width);
    // check for collisions
    this->DoCollisions();
//    // update particles
//    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
//    // update PowerUps
//    this->UpdatePowerUps(dt);
//
//    // check win condition
//    if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
//    {
//        this->ResetLevel();
//        this->ResetPlayer();
//        Effects->Chaos = true;
//        this->State = GAME_WIN;
//    }
}


void Game::ProcessInput(double dt) {
    auto player = pacman->gameObjects[pacman->GetCurrentModelIndex()];
    if (this->state == GAME_ACTIVE) {
        float speed = PLAYER_SPEED * static_cast<float>(dt);
        // Priority: UP > DOWN > RIGHT > LEFT
        // move player model
        if (this->keys[GLFW_KEY_UP] && permittedDirections.DIRECTION_UP) {
            permittedDirections = PermittedDirections();
            player->directions[0] = glm::vec3(1.0f, 0.0f, 0.0f);
            player->positions[0] += speed * player->directions[0];
        }
        else if (this->keys[GLFW_KEY_DOWN] && permittedDirections.DIRECTION_DOWN) {
            permittedDirections = PermittedDirections();
            player->directions[0] = glm::vec3(-1.0f, 0.0f, 0.0f);
            player->positions[0] += speed * player->directions[0];
        }
        else if (this->keys[GLFW_KEY_RIGHT] && permittedDirections.DIRECTION_RIGHT) {
            auto playerObb = player->GetTransformedBoundingBox(0);
            glm::vec3 pMax = playerObb.second;
            auto levelMatrixDim = this->Levels[this->level].levelMatrixDim;
            size_t columnDim = levelMatrixDim.second;

            permittedDirections = PermittedDirections();
            player->directions[0] = glm::vec3(0.0f, 0.0f, 1.0f);
            if (pMax.z >= static_cast<float>(columnDim)) {
                player->positions[0] = glm::vec3(player->positions[0].x, player->positions[0].y, 0.0f);
            }
            else {
                player->positions[0] += speed * player->directions[0];
            }
        }
        else if (this->keys[GLFW_KEY_LEFT] && permittedDirections.DIRECTION_LEFT) {
            auto playerObb = player->GetTransformedBoundingBox(0);
            glm::vec3 pMin = playerObb.first;
            auto levelMatrixDim = this->Levels[this->level].levelMatrixDim;
            size_t columnDim = levelMatrixDim.second;

            permittedDirections = PermittedDirections();
            player->directions[0] = glm::vec3(0.0f, 0.0f, -1.0f);
            if (pMin.z <= 0.0f) {
                player->positions[0] = glm::vec3(player->positions[0].x, player->positions[0].y, static_cast<float>(columnDim)-1.0f);
            }
            else {
                player->positions[0] += speed * player->directions[0];
            }
        }
        
    }
}

void Game::Render(double dt) {
    if (this->state == GAME_ACTIVE || this->state == GAME_WIN) {
        // begin rendering to postprocessing framebuffer
        //Effects->BeginRender();
        // draw level
        this->Levels[this->level].Draw();
        // draw player
        pacman->Draw(dt);
        ghost->Draw();
        //    // draw PowerUps
        //    for (PowerUp &powerUp : this->PowerUps)
        //        if (!powerUp.Destroyed)
        //            powerUp.Draw(*Renderer);
        //    // draw particles	
        //    Particles->Draw();
        //    // draw ball
        //    Ball->Draw(*Renderer);            
        //// end rendering to postprocessing framebuffer
        //Effects->EndRender();
        //// render postprocessing quad
        //Effects->Render(glfwGetTime());
        //// render text (don't include in postprocessing)
        //std::stringstream ss; ss << this->Lives;
        //Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
    }
    /*if (this->State == GAME_MENU)
    {
        Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2.0f, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, this->Height / 2.0f + 20.0f, 0.75f);
    }
    if (this->State == GAME_WIN)
    {
        Text->RenderText("You WON!!!", 320.0f, this->Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        Text->RenderText("Press ENTER to retry or ESC to quit", 130.0f, this->Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
    }*/
}

// collision detection
bool checkCollision(const obb& obb1, const obb& obb2);
glm::vec3 resolveCollision(const obb& playerObb, const obb& wallObb, PermittedDirections& permittedDirections);

void Game::DoCollisions() {
    auto player = pacman->gameObjects[pacman->GetCurrentModelIndex()];
    auto playerObb = player->GetTransformedBoundingBox(0);

    // CHECK COLLISION PLAYER-WALL
    GameObjectBase* mazeWall = this->Levels[this->level].mazeWall;
    size_t numInstancesMazeWall = mazeWall->GetNumInstances();
    for (size_t i = 0; i < numInstancesMazeWall; i++) {
        auto mazeWallObb = mazeWall->GetTransformedBoundingBox(i);
        bool collision = checkCollision(playerObb, mazeWallObb);
        if (collision) {
            LoggerManager::LogDebug("There was a collision between PLAYER and WALL number {}", i);
            // RESOLVE COLLISION PLAYER-WALL
            glm::vec3 correction = resolveCollision(playerObb, mazeWallObb, this->permittedDirections);
            player->positions[0] += correction; // Apply the correction vector
        }
    }

    // CHECK COLLISION PLAYER-DOT
    GameObjectBase* dot = this->Levels[this->level].dot;
    size_t numInstancesDot = dot->GetNumInstances();
    std::vector<glm::vec3> dotPositions = this->Levels[this->level].dotPositions;
    // When removing elements from an array during iteration, a reverse for loop 
    // helps avoid problems related to changing the length of the array as you walk through it
    for (int i = static_cast<int>(numInstancesDot) - 1; i >= 0; i--) {
        auto dotObb = dot->GetTransformedBoundingBox(i);
        bool collision = checkCollision(playerObb, dotObb);
        if (collision) {
            LoggerManager::LogDebug("There was a collision between PLAYER and DOT number {}", i);
            // RESOLVE COLLISION PLAYER-DOT
            dotPositions.erase(dotPositions.begin() + i);
            dot->positions.erase(dot->positions.begin() + i);
            dot->directions.erase(dot->directions.begin() + i);
            dot->rotations.erase(dot->rotations.begin() + i);
            dot->scaling.erase(dot->scaling.begin() + i);
            dot->SetNumInstances(dot->GetNumInstances() - 1);
        }
    }

    // CHECK COLLISION PLAYER-ENERGIZER
    GameObjectBase* energizer = this->Levels[this->level].energizer;
    size_t numInstancesEnergizer = energizer->GetNumInstances();
    std::vector<glm::vec3> energizerPositions = this->Levels[this->level].energizerPositions;
    for (int i = static_cast<int>(numInstancesEnergizer) - 1; i >= 0; i--) {
        auto energizerObb = energizer->GetTransformedBoundingBox(i);
        bool collision = checkCollision(playerObb, energizerObb);
        if (collision) {
            LoggerManager::LogDebug("There was a collision between PLAYER and ENERGIZER number {}", i);
            // RESOLVE COLLISION PLAYER-ENERGIZER
            energizerPositions.erase(energizerPositions.begin() + i);
            energizer->positions.erase(energizer->positions.begin() + i);
            energizer->directions.erase(energizer->directions.begin() + i);
            energizer->rotations.erase(energizer->rotations.begin() + i);
            energizer->scaling.erase(energizer->scaling.begin() + i);
            energizer->SetNumInstances(energizer->GetNumInstances() - 1);
        }
    }

}

// OBB - OBB collision detection in XZ plane
bool checkCollision(const obb& obb1, const obb& obb2) {
    glm::vec3 box1_min = obb1.first;
    glm::vec3 box1_max = obb1.second;
    glm::vec3 box2_min = obb2.first;
    glm::vec3 box2_max = obb2.second;

    // Check only on X and Z axes
    bool overlapX = box1_max.x >= box2_min.x && box1_min.x <= box2_max.x;
    bool overlapZ = box1_max.z >= box2_min.z && box1_min.z <= box2_max.z;

    return overlapX && overlapZ;
}

// Function to resolve collision in XZ plane
glm::vec3 resolveCollision(const obb& playerObb, const obb& wallObb, PermittedDirections& permittedDirections) {
    glm::vec3 playerMin = playerObb.first;
    glm::vec3 playerMax = playerObb.second;
    glm::vec3 wallMin = wallObb.first;
    glm::vec3 wallMax = wallObb.second;

    // Calculate the penetration depth on the X and Z axes
    float overlapX = std::min(playerMax.x - wallMin.x, wallMax.x - playerMin.x);
    float overlapZ = std::min(playerMax.z - wallMin.z, wallMax.z - playerMin.z);

    // Find the axis with the least penetration and use that value to resolve the collision
    if (overlapX < overlapZ) {
        if (playerMax.x < wallMax.x) {
            permittedDirections = PermittedDirections();
            permittedDirections.DIRECTION_UP = false;
            return glm::vec3(-overlapX, 0.0f, 0.0f);
        } else {
            permittedDirections = PermittedDirections();
            permittedDirections.DIRECTION_DOWN = false;
            return glm::vec3( overlapX, 0.0f, 0.0f);
        }
    }
    else {
        if (playerMax.z < wallMax.z) {
            permittedDirections = PermittedDirections();
            permittedDirections.DIRECTION_RIGHT = false;
            return glm::vec3(0.0f, 0.0f, -overlapZ);
        } else {
            permittedDirections = PermittedDirections();
            permittedDirections.DIRECTION_LEFT = false;
            return glm::vec3(0.0f, 0.0f, overlapZ);
        }
    }
}
