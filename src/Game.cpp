#include <windows.h>
#include <algorithm>
#include <sstream>
#include <irrKlang.h>
using namespace irrklang;

#include "Game.hpp"
#include "custom_types.hpp"
#include "FileSystem.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"
#include "GameObjectBase.hpp"
#include "GameObjectCustom.hpp"
#include "GameObjectFromModel.hpp"
#include "PacMan.hpp"
#include "Blinky.hpp"
#include "Clyde.hpp"
#include "Inky.hpp"
#include "Pinky.hpp"
#include "Utility.hpp"
#include "VulnerableGhost.hpp"
//#include "particle_generator.h"
//#include "post_processor.h"
//#include "text_renderer.h"


// Game-related State data
namespace {
    PacMan*               pacman;
    Blinky*               blinky;
    Clyde*                clyde;
    Inky*                 inky;
    Pinky*                pinky;
    VulnerableGhost*      vulnerableGhost;
    //ParticleGenerator*  Particles;
    //PostProcessor*      Effects;
    ISoundEngine*         soundEngine;
    ISoundSource*         pacmanChompSound;
    ISoundSource*         pacmanDeathSound;
    ISoundSource*         pacmanEatFruitSound;
    ISoundSource*         pacmanEatGhostSound;
    ISoundSource*         ghostNormalMove;
    ISoundSource*         ghostTurnBlue;
	ISoundSource*         currentPlayingSound;
    ISound*               currentSoundInstance = nullptr;
    //TextRenderer*       Text;
}

// Initial speed of the player
namespace {
    constexpr float PLAYER_SPEED = 7.5f;
}

namespace {

    // Function to detect OBB - OBB collision in XZ plane
    bool checkCollision(const CustomTypes::obb& obb1, const CustomTypes::obb& obb2) {
        const glm::vec3 box1_min = obb1.first;
        const glm::vec3 box1_max = obb1.second;
        const glm::vec3 box2_min = obb2.first;
        const glm::vec3 box2_max = obb2.second;

        // Check only on X and Z axes
        const bool overlapX = box1_max.x >= box2_min.x && box1_min.x <= box2_max.x;
        const bool overlapZ = box1_max.z >= box2_min.z && box1_min.z <= box2_max.z;

        return overlapX && overlapZ;
    }

    // Function to resolve collision in XZ plane
    glm::vec3 resolveCollision(const CustomTypes::obb& playerObb, const CustomTypes::obb& wallObb, PermittedDirections& permittedDirections) {
        const glm::vec3 playerMin = playerObb.first;
        const glm::vec3 playerMax = playerObb.second;
        const glm::vec3 wallMin = wallObb.first;
        const glm::vec3 wallMax = wallObb.second;

        // Calculate the penetration depth on the X and Z axes
        const float overlapX = std::min(playerMax.x - wallMin.x, wallMax.x - playerMin.x);
        const float overlapZ = std::min(playerMax.z - wallMin.z, wallMax.z - playerMin.z);

        // Find the axis with the least penetration and use that value to resolve the collision
        if (overlapX < overlapZ) {
            if (playerMax.x < wallMax.x) {
                permittedDirections = PermittedDirections();
                permittedDirections.DIRECTION_UP = false;
                return { -overlapX, 0.0f, 0.0f };
            }
            else {
                permittedDirections = PermittedDirections();
                permittedDirections.DIRECTION_DOWN = false;
                return { overlapX, 0.0f, 0.0f };
            }
        }
        else {
            if (playerMax.z < wallMax.z) {
                permittedDirections = PermittedDirections();
                permittedDirections.DIRECTION_RIGHT = false;
                return { 0.0f, 0.0f, -overlapZ };
            }
            else {
                permittedDirections = PermittedDirections();
                permittedDirections.DIRECTION_LEFT = false;
                return { 0.0f, 0.0f, overlapZ };
            }
        }
    }

    // Function to play sounds in loop, with diversity control compared to the current sound
    void playSoundIfChanged(ISoundSource* newSound, const bool loop = false) {
        if (currentPlayingSound != newSound) {
            // Stop the current sound
            if (currentSoundInstance) {
                currentSoundInstance->stop();
                currentSoundInstance = nullptr;
            }
            // Start the new sound
            currentSoundInstance = soundEngine->play2D(newSound, loop, false, true);
            if (currentSoundInstance) {
                currentSoundInstance->setIsPaused(false); // Make sure it starts
            }
            currentPlayingSound = newSound; // Update the current sound
        }
    }

    // Function to stop the current sound
    void stopCurrentSound() {
        if (currentSoundInstance) {
            currentSoundInstance->stop();
            currentSoundInstance = nullptr;
        }
        //currentPlayingSound->drop();
    }

}

Game::Game(const unsigned int width, const unsigned int height)
    : state(GAME_ACTIVE), keys(), keysProcessed(), width(width),
      height(height), level(0), lives(3), cameraPos(), cameraAt(),
      up(),cameraDir(),cameraSide(),cameraUp() { }

Game::~Game() {
    delete pacman;
    delete blinky;
    delete clyde;
    delete inky;
    delete pinky;
    delete vulnerableGhost;
    /*delete Particles;
    delete Effects;*/
    /*pacmanChompSound->drop();
    pacmanDeathSound->drop();
    pacmanEatFruitSound->drop();
    pacmanEatGhostSound->drop();
    ghostNormalMove->drop();
    ghostTurnBlue->drop();*/
    //soundEngine->drop();
    /*delete Text;*/
}

void Game::Init() {
    /// Load Shaders
    ResourceManager::LoadShader("./shaders/mazeWall.vs",  "./shaders/mazeWall.fs",  nullptr, "mazeWallShader");
    ResourceManager::LoadShader("./shaders/mazeFloor.vs", "./shaders/mazeFloor.fs", nullptr, "mazeFloorShader");
    ResourceManager::LoadShader("./shaders/dot.vs",       "./shaders/dot.fs",       nullptr, "dotShader");
    ResourceManager::LoadShader("./shaders/dot.vs",       "./shaders/dot.fs",       nullptr, "energizerShader");
    ResourceManager::LoadShader("./shaders/pacman.vs",    "./shaders/pacman.fs",    nullptr, "pacmanShader");
    ResourceManager::LoadShader("./shaders/ghost.vs",     "./shaders/ghost.fs",     nullptr, "ghostShader");
    ResourceManager::LoadShader("./shaders/bonusSymbol.vs","./shaders/bonusSymbol.fs",nullptr,"bonusSymbolShader");
    ResourceManager::LoadShader("./shaders/pacman.vs",    "./shaders/pacman.fs",    nullptr, "lifeCounterShader");
    /*ResourceManager::LoadShader("particle.vs", "particle.fs", nullptr, "particle");
    ResourceManager::LoadShader("post_processing.vs", "post_processing.fs", nullptr, "postprocessing");*/

    /// Configure Shaders
    // Insert uniform variable in vertex shader(only global variables, i.e. the same for all shaders)
    this->cameraPos = glm::vec3( -17.0, 22.5, 15.0);
    this->cameraAt  = glm::vec3(  10.0,  1.0, 15.0);
    this->up        = glm::vec3(   0.0,  1.0,  0.0);
    this->cameraDir = glm::normalize(cameraPos - cameraAt);
    this->cameraSide = glm::normalize(glm::cross(up, cameraDir));
    this->cameraUp = glm::normalize(glm::cross(cameraDir, cameraSide));
    const glm::mat4 view = glm::lookAt(cameraPos, cameraAt, cameraUp);
    const glm::mat4 projection = glm::perspective(glm::radians(35.0f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 55.0f);
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
    ResourceManager::GetShader("bonusSymbolShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("bonusSymbolShader").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("lifeCounterShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("lifeCounterShader").Use().SetMatrix4("projection", projection);
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
    ResourceManager::GetShader("bonusSymbolShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("bonusSymbolShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("bonusSymbolShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.7f, 0.7f, 0.7f));
    ResourceManager::GetShader("bonusSymbolShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
    ResourceManager::GetShader("bonusSymbolShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("bonusSymbolShader").Use().SetFloat("material.shininess", 32.0f);
    ResourceManager::GetShader("lifeCounterShader").Use().SetVector3f("viewPos", cameraPos);
    ResourceManager::GetShader("lifeCounterShader").Use().SetVector3f("dirLight.direction", glm::normalize(cameraAt - cameraPos));
    ResourceManager::GetShader("lifeCounterShader").Use().SetVector3f("dirLight.ambient", glm::vec3(0.7f, 0.7f, 0.7f));
    ResourceManager::GetShader("lifeCounterShader").Use().SetVector3f("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
    ResourceManager::GetShader("lifeCounterShader").Use().SetVector3f("dirLight.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    ResourceManager::GetShader("lifeCounterShader").Use().SetFloat("material.shininess", 32.0f);

    /// Load Textures
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/wall_diffuse_360.png").c_str(), "mazeWallDiffuseTexture");
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/wall_specular_360.png").c_str(), "mazeWallSpecularTexture");
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/floor_diffuse_360.png").c_str(), "mazeFloorDiffuseTexture");
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/floor_specular_360.png").c_str(), "mazeFloorSpecularTexture");

    /// Load Models
    ResourceManager::LoadModel("../res/objects/powerup/coin/coin.obj", "dotModel");
    ResourceManager::LoadModel("../res/objects/powerup/coin/coin.obj", "energizerModel");
    ResourceManager::LoadModel("../res/objects/powerup/cherries/cherries.obj", "cherriesModel");
    ResourceManager::LoadModel("../res/objects/powerup/cherries/cherries.obj", "cherriesFruitCounterModel");
    ResourceManager::LoadModel("../res/objects/pacman/pacman7/pacman7.obj", "lifeCounterPacmanModel");

    /// Load Levels
    const auto levelOne = new GameLevel();
    levelOne->Load(FileSystem::getPath("../res/levels/one.lvl").c_str());
    this->Levels.push_back(levelOne);
    this->level = 0;

    /// Load and Configure Music Tracks
    soundEngine = createIrrKlangDevice();
    // Play a short, irrelevant sound at the start of the game to force IrrKlang to initialize.
    // This trick prepare the audio engine and eliminates the delay when a significant sound is first played.
    soundEngine->play2D(FileSystem::getPath("../res/sounds/17. Silence.flac").c_str(),
                        false, 
                        false, 
                        true)->stop();
    // Preload audio tracks
    pacmanChompSound    = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/03. PAC-MAN - Eating The Pac-dots.flac").c_str());
    ghostNormalMove     = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/06. Ghost - Normal Move.flac").c_str());
    pacmanEatFruitSound = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/11. PAC-MAN - Eating The Fruit.flac").c_str());
    ghostTurnBlue       = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/12. Ghost - Turn to Blue.flac").c_str());
    pacmanEatGhostSound = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/13. PAC-MAN - Eating The Ghost.flac").c_str());
    pacmanDeathSound    = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/15. Fail.flac").c_str());

    // Set a default volume for each source
    pacmanChompSound   ->setDefaultVolume(1.0f);
    ghostNormalMove    ->setDefaultVolume(1.0f);
    pacmanEatFruitSound->setDefaultVolume(1.0f);
    ghostTurnBlue      ->setDefaultVolume(1.0f);
    pacmanEatGhostSound->setDefaultVolume(1.0f);
    pacmanDeathSound   ->setDefaultVolume(1.0f);
    

    /// Configure Game Objects
    pacman = new PacMan();
    const auto levelMatrixDim = this->Levels[this->level]->levelMatrixDim;
    blinky = new Blinky(levelMatrixDim);
    clyde  = new Clyde(levelMatrixDim);
    inky   = new Inky(levelMatrixDim);
    pinky  = new Pinky(levelMatrixDim);
    vulnerableGhost = new VulnerableGhost(blinky,clyde, inky, pinky, levelMatrixDim);
}

/// TODO:Introdurre stampa a schermo del punteggio


void Game::ProcessInput(const double dt) {
    const auto player = pacman->gameObjects[pacman->GetCurrentModelIndex()];
    //TODO:Capire e risolvere il fatto che Pac-Man alle volte (solo dopo una collisione) ha delle direzioni bloccate quando in realta non lo sono(Bug Fix #2)
    if (this->state == GAME_ACTIVE) {
        const float speed = PLAYER_SPEED * static_cast<float>(dt);
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
            const auto playerObb = player->GetTransformedBoundingBox(0);
            const glm::vec3 pMax = playerObb.second;
            const auto levelMatrixDim = this->Levels[this->level]->levelMatrixDim;
            const size_t columnDim = levelMatrixDim.second;

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
            const auto playerObb = player->GetTransformedBoundingBox(0);
            const glm::vec3 pMin = playerObb.first;
            const auto levelMatrixDim = this->Levels[this->level]->levelMatrixDim;
            const size_t columnDim = levelMatrixDim.second;

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

void Game::Update(const double dt) {
    // update objects
    const auto mazeWall = this->Levels[this->level]->mazeWall;
    if (vulnerableGhost->IsActive()) {
        vulnerableGhost->Move(dt, mazeWall);
    }
    else {
        if (blinky->IsAlive()) blinky->Move(dt, mazeWall);
        if (clyde->IsAlive())  clyde->Move(dt, mazeWall);
        if (inky->IsAlive())   inky->Move(dt, mazeWall);
        if (pinky->IsAlive())  pinky->Move(dt, mazeWall);
    }
    // check for collisions
    this->DoCollisions(dt);
    //    // update particles
    //    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
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

void Game::Render(const double dt) const {
    if (this->state == GAME_ACTIVE || this->state == GAME_WIN) {
        // begin rendering to postprocessing framebuffer
        //Effects->BeginRender();
        // draw level
        this->Levels[this->level]->Draw(dt);
        // draw player
        pacman->Draw(dt);
        if (vulnerableGhost->IsActive() && vulnerableGhost->GetCurrentGameObject()->GetNumInstances() > 0) {

            vulnerableGhost->Draw(dt);
            playSoundIfChanged(ghostTurnBlue, true);
        } else {
            if (blinky->IsAlive()) blinky->Draw(dt);
            if (clyde->IsAlive())  clyde ->Draw(dt);
            if (inky->IsAlive())   inky  ->Draw(dt);
            if (pinky->IsAlive())  pinky ->Draw(dt);

            if (blinky->IsAlive() || clyde->IsAlive() || inky->IsAlive() || pinky->IsAlive()) {
                playSoundIfChanged(ghostNormalMove, true);
            }
            else {
                stopCurrentSound();
            }
        }
        //    // draw particles	
        //    Particles->Draw();            
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
void Game::DoCollisions(double dt) {
    auto player = pacman->gameObjects[pacman->GetCurrentModelIndex()];
    auto playerObb = player->GetTransformedBoundingBox(0);

    // CHECK COLLISION PLAYER-WALL
    GameObjectBase* mazeWall = this->Levels[this->level]->mazeWall;
    size_t numInstancesMazeWall = mazeWall->GetNumInstances();
    for (size_t i = 0; i < numInstancesMazeWall; i++) {
        auto mazeWallObb = mazeWall->GetTransformedBoundingBox(i);
        if (checkCollision(playerObb, mazeWallObb)) {
            LoggerManager::LogDebug("There was a collision between PLAYER and WALL number {}", i);
            // RESOLVE COLLISION PLAYER-WALL
            glm::vec3 correction = resolveCollision(playerObb, mazeWallObb, this->permittedDirections);
            player->positions[0] += correction; // Apply the correction vector
        }
    }

    this->chompTimer += dt;
    // CHECK COLLISION PLAYER-DOT
    GameObjectBase* dot = this->Levels[this->level]->dot;
    size_t numInstancesDot = dot->GetNumInstances();
    std::vector<glm::vec3> dotPositions = this->Levels[this->level]->dotPositions;
    // When removing elements from an array during iteration, a reverse for loop 
    // helps avoid problems related to changing the length of the array as you walk through it
    for (int i = static_cast<int>(numInstancesDot) - 1; i >= 0; i--) {
        auto dotObb = dot->GetTransformedBoundingBox(i);
        if (checkCollision(playerObb, dotObb)) {
            if (chompTimer >= CHOMP_INTERVAL) {
                soundEngine->play2D(pacmanChompSound, false);
                chompTimer = 0.0;
            }
            LoggerManager::LogDebug("There was a collision between PLAYER and DOT number {}", i);
            // RESOLVE COLLISION PLAYER-DOT
            dotPositions.erase(dotPositions.begin() + i);
            dot->positions.erase(dot->positions.begin() + i);
            dot->directions.erase(dot->directions.begin() + i);
            dot->rotations.erase(dot->rotations.begin() + i);
            dot->scaling.erase(dot->scaling.begin() + i);
            dot->UpdateNumInstance();
        }
    }

    // CHECK COLLISION PLAYER-ENERGIZER
    GameObjectBase* energizer = this->Levels[this->level]->energizer;
    size_t numInstancesEnergizer = energizer->GetNumInstances();
    std::vector<glm::vec3> energizerPositions = this->Levels[this->level]->energizerPositions;
    for (int i = static_cast<int>(numInstancesEnergizer) - 1; i >= 0; i--) {
        auto energizerObb = energizer->GetTransformedBoundingBox(i);
        if (checkCollision(playerObb, energizerObb)) {
            if (chompTimer >= CHOMP_INTERVAL) {
                soundEngine->play2D(pacmanChompSound, false);
                chompTimer = 0.0;
            }
            LoggerManager::LogDebug("There was a collision between PLAYER and ENERGIZER number {}", i);
            // RESOLVE COLLISION PLAYER-ENERGIZER
            energizerPositions.erase(energizerPositions.begin() + i);
            energizer->positions.erase(energizer->positions.begin() + i);
            energizer->directions.erase(energizer->directions.begin() + i);
            energizer->rotations.erase(energizer->rotations.begin() + i);
            energizer->scaling.erase(energizer->scaling.begin() + i);
            energizer->UpdateNumInstance();
            vulnerableGhost->SetActive(true);
        }
    }

    // CHECK COLLISION PLAYER-BONUS_SYMBOL
    GameObjectBase* bonusSymbol = this->Levels[this->level]->bonusSymbol;
    size_t numInstancesBonusSymbol = bonusSymbol->GetNumInstances();
    for (int i = static_cast<int>(numInstancesBonusSymbol) - 1; i >= 0; i--) {
        auto bonusSymbolObb = bonusSymbol->GetTransformedBoundingBox(i);
        if (checkCollision(playerObb, bonusSymbolObb)) {
            soundEngine->play2D(pacmanEatFruitSound, false);
            LoggerManager::LogDebug("There was a collision between PLAYER and BONUS_SYMBOL number {}", i);
            // RESOLVE COLLISION PLAYER-BONUS_SYMBOL
            if (this->Levels[this->level]->GetSymbolActive() == 2) {
                this->Levels[this->level]->SetBonusSymbolPosition(glm::vec3(-2.0f, 0.0f, -2.0f));
                this->Levels[this->level]->SetPlayerTakeBonusSymbol(true);
                this->Levels[this->level]->SetSecondActivationTimeAccumulator(11.0f);
            } else if (this->Levels[this->level]->GetSymbolActive() == 1) {
                this->Levels[this->level]->SetBonusSymbolPosition(glm::vec3(-2.0f, 0.0f, -2.0f));
                this->Levels[this->level]->SetPlayerTakeBonusSymbol(true);
                this->Levels[this->level]->SetFirstActivationTimeAccumulator(11.0f);
            }
        }
    }

    // CHECK COLLISION PLAYER-GHOSTS

    // CHECK COLLISION PLAYER-VULNERABLE_GHOST
    if (vulnerableGhost->IsActive()) {
        for (int j = static_cast<int>(vulnerableGhost->GetCurrentGameObject()->GetNumInstances()) - 1 ; j >= 0; j--) {
            auto currenGameObjectVulnerableGhost = vulnerableGhost->GetCurrentGameObject();
            auto currenGameObjectVulnerableGhostObb = currenGameObjectVulnerableGhost->GetTransformedBoundingBox(j);
            if (checkCollision(playerObb, currenGameObjectVulnerableGhostObb)) {
                soundEngine->play2D(pacmanEatGhostSound, false);
                LoggerManager::LogDebug("There was a collision between PLAYER and VULNERABLE_GHOST");
                // RESOLVE COLLISION PLAYER-VULNERABLE_GHOST
                if (vulnerableGhost->ghostMapping.blinkyIndex == j) blinky->SetAlive(false);
                if (vulnerableGhost->ghostMapping.clydeIndex == j) clyde->SetAlive(false);
                if (vulnerableGhost->ghostMapping.inkyIndex == j) inky->SetAlive(false);
                if (vulnerableGhost->ghostMapping.pinkyIndex == j) pinky->SetAlive(false);
                vulnerableGhost->RemoveAnInstace(j);
            }
        }
        
    } else {
        auto lifeCounter = this->Levels[this->level]->lifeCounter;
        //TODO:Gestire multi-collisioni se Pac-man e al centro che portano a perdere tutte e 3 le vite in un singolo colpo(Bug Fix #1)

        // CHECK COLLISION PLAYER-BLINKY
    	if (blinky->IsAlive()) {
            auto blinkyObb = blinky->gameObject->GetTransformedBoundingBox(0);
            if (checkCollision(playerObb, blinkyObb)) {
                soundEngine->play2D(pacmanDeathSound, false);
                LoggerManager::LogDebug("There was a collision between PLAYER and BLINKY");
                // RESOLVE COLLISION PLAYER-BLINKY
                if (this->lives > 1) {
                    this->lives--;
                    pacman->gameObjects[pacman->GetCurrentModelIndex()]->positions[0] = glm::vec3(7.5f, 0.0f, 13.5f);
                    pacman->UpdateOtherGameObjects();
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                }
                else {
                    this->state = GAME_DEFEAT;
                }
            }
        }

        // CHECK COLLISION PLAYER-CLYDE
        if (clyde->IsAlive()) {
            auto clydeObb = clyde->gameObject->GetTransformedBoundingBox(0);
            if (checkCollision(playerObb, clydeObb)) {
                soundEngine->play2D(pacmanDeathSound, false);
                LoggerManager::LogDebug("There was a collision between PLAYER and CLYDE");
                // RESOLVE COLLISION PLAYER-CLYDE
                if (this->lives > 1) {
                    this->lives--;
                    pacman->gameObjects[pacman->GetCurrentModelIndex()]->positions[0] = glm::vec3(7.5f, 0.0f, 13.5f);
                    pacman->UpdateOtherGameObjects();
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                }
                else {
                    this->state = GAME_DEFEAT;
                }
            }
        }
        

        // CHECK COLLISION PLAYER-INKY
        if (inky->IsAlive()) {
            auto inkyObb = inky->gameObject->GetTransformedBoundingBox(0);
            if (checkCollision(playerObb, inkyObb)) {
                soundEngine->play2D(pacmanDeathSound, false);
                LoggerManager::LogDebug("There was a collision between PLAYER and INKY");
                // RESOLVE COLLISION PLAYER-INKY
                if (this->lives > 1) {
                    this->lives--;
                    pacman->gameObjects[pacman->GetCurrentModelIndex()]->positions[0] = glm::vec3(7.5f, 0.0f, 13.5f);
                    pacman->UpdateOtherGameObjects();
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                }
                else {
                    this->state = GAME_DEFEAT;
                }
            }
        }
        

        // CHECK COLLISION PLAYER-PINKY
        if (pinky->IsAlive()) {
            auto pinkyObb = pinky->gameObject->GetTransformedBoundingBox(0);
            if (checkCollision(playerObb, pinkyObb)) {
                soundEngine->play2D(pacmanDeathSound, false);
                LoggerManager::LogDebug("There was a collision between PLAYER and PINKY");
                // RESOLVE COLLISION PLAYER-INKY
                if (this->lives > 1) {
                    this->lives--;
                    pacman->gameObjects[pacman->GetCurrentModelIndex()]->positions[0] = glm::vec3(7.5f, 0.0f, 13.5f);
                    pacman->UpdateOtherGameObjects();
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                }
                else {
                    this->state = GAME_DEFEAT;
                }
            }
        }

    }

}
