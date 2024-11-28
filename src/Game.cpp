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
#include "TextRenderer.hpp"
#include "PostProcessor.hpp"


// Game-related State data
namespace {
    PacMan*               pacman;
    Blinky*               blinky;
    Clyde*                clyde;
    Inky*                 inky;
    Pinky*                pinky;
    VulnerableGhost*      vulnerableGhost;
    //ParticleGenerator*  Particles;
    ISoundEngine*         soundEngine;
    ISoundSource*         pacmanChompSound;
    ISoundSource*         pacmanDeathSound;
    ISoundSource*         pacmanEatFruitSound;
    ISoundSource*         pacmanEatGhostSound;
    ISoundSource*         ghostNormalMoveSound;
    ISoundSource*         ghostTurnBlueSound;
    ISoundSource*         pacmanEatsAllGhostsSound;
    ISoundSource*         victorySound;
	ISoundSource*         currentPlayingSound;
    ISound*               currentSoundInstance = nullptr;
    TextRenderer*         text;
    PostProcessor*        postProcessor;

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
        const glm::vec3 wallMin   = wallObb.first;
        const glm::vec3 wallMax   = wallObb.second;

        // Calculating overlaps
        const float overlapX = std::min(playerMax.x, wallMax.x) - std::max(playerMin.x, wallMin.x);
        const float overlapZ = std::min(playerMax.z, wallMax.z) - std::max(playerMin.z, wallMin.z);

        // Tolerance for comparison
        constexpr float epsilon = 0.001f;

        // Resets all values ​​to true so that only one direction is blocked at a time
        permittedDirections = PermittedDirections();

        // Determine the axis with the least penetration
        if (std::abs(overlapX - overlapZ) < epsilon) {

            // Special case Nearly equal overlaps
            if (playerMax.x < wallMax.x) {
                permittedDirections.DIRECTION_UP = false;
                return { -overlapX, 0.0f, 0.0f };
            }
            else {
                permittedDirections.DIRECTION_DOWN = false;
                return { overlapX, 0.0f, 0.0f };
            }

        }
        else if (overlapX < overlapZ) {

            if (playerMax.x < wallMax.x) {
                permittedDirections.DIRECTION_UP = false;
                return { -overlapX, 0.0f, 0.0f };
            }
            else {
                permittedDirections.DIRECTION_DOWN = false;
                return { overlapX, 0.0f, 0.0f };
            }

        }
        else {

            if (playerMax.z < wallMax.z) {
                permittedDirections.DIRECTION_RIGHT = false;
                return { 0.0f, 0.0f, -overlapZ };
            }
            else {
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

    // Function to calculate points
    int calculatePoints(const int ghostCounter) {
        return 200 * (1 << (ghostCounter - 1)); // 200, 400, 800, 1600
    }

}

Game::Game(const unsigned int width, const unsigned int height)
    : state(GAME_ACTIVE), keys(), keysProcessed(), width(width),
      height(height), level(0), lives(3), score(0), cameraPos(),
      cameraAt(), up(),cameraDir(),cameraSide(),cameraUp() { }

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
    ghostNormalMoveSound->drop();
    ghostTurnBlueSound->drop();*/
    //soundEngine->drop();
    delete text;
    delete postProcessor;
}

void Game::Init() {
    /// Load Shaders
    ResourceManager::LoadShader("./shaders/mazeWall.vs",   "./shaders/mazeWall.fs",   nullptr, "mazeWallShader");
    ResourceManager::LoadShader("./shaders/mazeFloor.vs",  "./shaders/mazeFloor.fs",  nullptr, "mazeFloorShader");
    ResourceManager::LoadShader("./shaders/dot.vs",        "./shaders/dot.fs",        nullptr, "dotShader");
    ResourceManager::LoadShader("./shaders/dot.vs",        "./shaders/dot.fs",        nullptr, "energizerShader");
    ResourceManager::LoadShader("./shaders/pacman.vs",     "./shaders/pacman.fs",     nullptr, "pacmanShader");
    ResourceManager::LoadShader("./shaders/ghost.vs",      "./shaders/ghost.fs",      nullptr, "ghostShader");
    ResourceManager::LoadShader("./shaders/bonusSymbol.vs","./shaders/bonusSymbol.fs",nullptr, "bonusSymbolShader");
    ResourceManager::LoadShader("./shaders/pacman.vs",     "./shaders/pacman.fs",     nullptr, "lifeCounterShader");
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

    /// Configure Game Objects
    pacman = new PacMan();
    const auto levelMatrixDim = this->Levels[this->level]->levelMatrixDim;
    blinky = new Blinky(levelMatrixDim);
    clyde = new Clyde(levelMatrixDim);
    inky = new Inky(levelMatrixDim);
    pinky = new Pinky(levelMatrixDim);
    vulnerableGhost = new VulnerableGhost(blinky, clyde, inky, pinky, levelMatrixDim);

    /// Load and Configure Music Tracks
    soundEngine = createIrrKlangDevice();
    // Play a short, irrelevant sound at the start of the game to force IrrKlang to initialize.
    // This trick prepare the audio engine and eliminates the delay when a significant sound is first played.
    soundEngine->play2D(FileSystem::getPath("../res/sounds/17. Silence.flac").c_str(),
                        false, 
                        false, 
                        true)->stop();
    // Preload audio tracks
    pacmanChompSound         = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/03. PAC-MAN - Eating The Pac-dots.flac").c_str());
    pacmanEatsAllGhostsSound = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/05. PAC-MAN - All the ghosts have been eaten.flac").c_str());
    ghostNormalMoveSound     = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/06. Ghost - Normal Move.flac").c_str());
    pacmanEatFruitSound      = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/11. PAC-MAN - Eating The Fruit.flac").c_str());
    ghostTurnBlueSound       = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/12. Ghost - Turn to Blue.flac").c_str());
    pacmanEatGhostSound      = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/13. PAC-MAN - Eating The Ghost.flac").c_str());
    pacmanDeathSound         = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/15. Fail.flac").c_str());
    victorySound             = soundEngine->addSoundSourceFromFile(FileSystem::getPath("../res/sounds/16. Coffee Break Music.flac").c_str());

    // Set a default volume for each source
    pacmanChompSound        ->setDefaultVolume(1.0f);
    ghostNormalMoveSound    ->setDefaultVolume(1.0f);
    pacmanEatFruitSound     ->setDefaultVolume(1.0f);
    ghostTurnBlueSound      ->setDefaultVolume(1.0f);
    pacmanEatGhostSound     ->setDefaultVolume(1.0f);
    pacmanDeathSound        ->setDefaultVolume(1.0f);
    pacmanEatsAllGhostsSound->setDefaultVolume(1.0f);
    victorySound            ->setDefaultVolume(1.0f);

    /// Configure render-specific objects
    text = new TextRenderer(this->width, this->height);
    text->Load(FileSystem::getPath("../res/fonts/eight_bit_dragon.ttf"), 32);

    postProcessor = new PostProcessor(this->width, this->height, true, 4);
}

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
        /*LoggerManager::LogInfo("----------------------------");
        LoggerManager::LogInfo("DIRECTION_UP:{}", permittedDirections.DIRECTION_UP);
        LoggerManager::LogInfo("DIRECTION_DOWN:{}", permittedDirections.DIRECTION_DOWN);
        LoggerManager::LogInfo("DIRECTION_LEFT:{}", permittedDirections.DIRECTION_LEFT);
        LoggerManager::LogInfo("DIRECTION_RIGHT:{}", permittedDirections.DIRECTION_RIGHT);
        LoggerManager::LogInfo("----------------------------");*/
    }
}

void Game::Update(const double dt) {

    if (this->state == GAME_ACTIVE) {
        // update objects
        if (pacman->IsInvulnerable()) {
            this->spawnProctectionTimeAccumulator += dt;
            if (this->spawnProctectionTimeAccumulator >= this->SPAWN_PROTECTION_TIME_LIMIT) {
                pacman->SetInvulnerable(false);
                this->spawnProctectionTimeAccumulator = 0.0f;
            }
        }

        const auto mazeWall = this->Levels[this->level]->mazeWall;
        if (vulnerableGhost->IsActive()) {
            vulnerableGhost->Move(dt, mazeWall);
        }
        else {
            if (blinky->IsAlive()) {
                blinky->Move(dt, mazeWall);
            }
            else if (blinky->ShouldRespawn(dt)) {
                blinky->ResetGameObjectProperties();
                blinky->SetAlive(true);
                blinky->Move(dt, mazeWall);
                vulnerableGhost->AddAnInstance(blinky->gameObject->positions[0],
                                               blinky->gameObject->directions[0],
                                               blinky->gameObject->rotations[0],
                                               blinky->gameObject->scaling[0]);
            }

            if (clyde->IsAlive()) {
                clyde->Move(dt, mazeWall);
            }
            else if (clyde->ShouldRespawn(dt)) {
                clyde->ResetGameObjectProperties();
                clyde->SetAlive(true);
                clyde->Move(dt, mazeWall);
                vulnerableGhost->AddAnInstance(clyde->gameObject->positions[0],
                                               clyde->gameObject->directions[0],
                                               clyde->gameObject->rotations[0],
                                               clyde->gameObject->scaling[0]);
            }

            if (inky->IsAlive()) {
                inky->Move(dt, mazeWall);
            }
            else if (inky->ShouldRespawn(dt)) {
                inky->ResetGameObjectProperties();
                inky->SetAlive(true);
                inky->Move(dt, mazeWall);
                vulnerableGhost->AddAnInstance(inky->gameObject->positions[0],
                                               inky->gameObject->directions[0],
                                               inky->gameObject->rotations[0],
                                               inky->gameObject->scaling[0]);
            }

            if (pinky->IsAlive()) {
                pinky->Move(dt, mazeWall);
            }
            else if (pinky->ShouldRespawn(dt)) {
                pinky->ResetGameObjectProperties();
                pinky->SetAlive(true);
                pinky->Move(dt, mazeWall);
                vulnerableGhost->AddAnInstance(pinky->gameObject->positions[0],
                                               pinky->gameObject->directions[0],
                                               pinky->gameObject->rotations[0],
                                               pinky->gameObject->scaling[0]);
            }

        }
        // check for collisions
        this->DoCollisions(dt);
        //    // update particles
        //    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
    }
    
    // check win condition
    if (this->state == GAME_ACTIVE && this->Levels[this->level]->IsCompleted()) {
        /*this->ResetLevel();
        this->ResetPlayer();*/
        this->state = GAME_WIN;
        soundEngine->play2D(victorySound, false);
    }
}

void Game::Render(const double dt) const {
    if (this->state == GAME_ACTIVE || this->state == GAME_WIN || this->state == GAME_DEFEAT) {
        // begin rendering to postprocessing framebuffer
        postProcessor->BeginRender();
        // draw level
        this->Levels[this->level]->Draw(dt);
        // draw player
        pacman->Draw(dt);
        if (vulnerableGhost->IsActive()) {
            vulnerableGhost->Draw(dt);

            if (vulnerableGhost->GetCurrentGameObject()->GetNumInstances() > 0) {
                playSoundIfChanged(ghostTurnBlueSound, true);
            } else {
                stopCurrentSound();
            }
            
        } else {
            if (blinky->IsAlive()) blinky->Draw(dt);
            if (clyde->IsAlive())  clyde ->Draw(dt);
            if (inky->IsAlive())   inky  ->Draw(dt);
            if (pinky->IsAlive())  pinky ->Draw(dt);

            if (blinky->IsAlive() || clyde->IsAlive() || inky->IsAlive() || pinky->IsAlive()) {
                playSoundIfChanged(ghostNormalMoveSound, true);
            }
            else {
                stopCurrentSound();
            }
        }
        
        //    // draw particles	
        //    Particles->Draw();            
        // end rendering to postprocessing framebuffer
        postProcessor->Render(dt);
        postProcessor->EndRender();
        //// render postprocessing quad
        //Effects->Render(glfwGetTime());

        // render text (don't include in postprocessing)
        const float widthFloat = static_cast<float>(this->width);
        const std::string scoreString = std::to_string(this->score);
        text->RenderText("1UP",        (widthFloat / 2.0f) - (widthFloat /  5.0f), 10, 1.0f);
        text->RenderText(scoreString,    (widthFloat / 2.0f) - (widthFloat /  5.0f), 50, 1.0f);
        text->RenderText("HIGH SCORE", (widthFloat / 2.0f) - (widthFloat / 20.0f), 10, 1.0f);
        text->RenderText(scoreString,    (widthFloat / 2.0f) - (widthFloat / 20.0f), 50, 1.0f);
    }
    /*if (this->State == GAME_MENU)
    {
        Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2.0f, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, this->Height / 2.0f + 20.0f, 0.75f);
    }*/
    if (this->state == GAME_WIN) {
        const float widthFloat = static_cast<float>(this->width);
        const float heightFloat = static_cast<float>(this->height);
        text->RenderText("You WON!!!",        (widthFloat / 2.0f) - (widthFloat / 20.0f),         (heightFloat / 2.0f) - (heightFloat / 9.0f),         1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        text->RenderText("Press ESC to quit", (widthFloat / 2.0f) - (widthFloat / 20.0f) - 70.0f, (heightFloat / 2.0f) - (heightFloat / 9.0f) + 40.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        stopCurrentSound();
    }
    if (this->state == GAME_DEFEAT) {
        const float widthFloat = static_cast<float>(this->width);
        const float heightFloat = static_cast<float>(this->height);
        text->RenderText("You LOST!!!",       (widthFloat / 2.0f) - (widthFloat / 20.0f),         (heightFloat / 2.0f) - (heightFloat / 9.0f),         1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        text->RenderText("Press ESC to quit", (widthFloat / 2.0f) - (widthFloat / 20.0f) - 60.0f, (heightFloat / 2.0f) - (heightFloat / 9.0f) + 40.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        stopCurrentSound();
    }
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
            this->score += 10;
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
            this->score += 50;
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
            this->score += 100;
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
                if (vulnerableGhost->ghostMapping.clydeIndex  == j)  clyde->SetAlive(false);
                if (vulnerableGhost->ghostMapping.inkyIndex   == j)   inky->SetAlive(false);
                if (vulnerableGhost->ghostMapping.pinkyIndex  == j)  pinky->SetAlive(false);
                vulnerableGhost->RemoveAnInstace(j);
                this->ghostCounter++;
                int points = calculatePoints(this->ghostCounter);
                this->score += points;
                if (this->ghostCounter == 4) {
                    soundEngine->play2D(pacmanEatsAllGhostsSound, false);
                }
            }
        }
        
    } else {
        this->ghostCounter = 0;
        auto lifeCounter = this->Levels[this->level]->lifeCounter;
        //TODO:Gestire multi-collisioni se Pac-man e al centro che portano a perdere tutte e 3 le vite in un singolo colpo(Bug Fix #1)

        if (!pacman->IsInvulnerable()) {

            // CHECK COLLISION PLAYER-BLINKY
            if (blinky->IsAlive()) {
                auto blinkyObb = blinky->gameObject->GetTransformedBoundingBox(0);
                if (checkCollision(playerObb, blinkyObb)) {
                    soundEngine->play2D(pacmanDeathSound, false);
                    LoggerManager::LogDebug("There was a collision between PLAYER and BLINKY");
                    // RESOLVE COLLISION PLAYER-BLINKY
                    this->lives--;
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                    pacman->SetInvulnerable(true);
                    if (this->lives == 0) {
                        this->state = GAME_DEFEAT;
                    }
                    else {
                        ResetPlayerAndGhosts();
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
                    this->lives--;
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                    pacman->SetInvulnerable(true);
                    if (this->lives == 0) {
                        this->state = GAME_DEFEAT;
                    }
                    else {
                        ResetPlayerAndGhosts();
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
                    this->lives--;
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                    pacman->SetInvulnerable(true);
                    if (this->lives == 0) {
                        this->state = GAME_DEFEAT;
                    }
                    else {
                        ResetPlayerAndGhosts();
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
                    this->lives--;
                    lifeCounter->positions.erase(lifeCounter->positions.begin() + this->lives);
                    lifeCounter->directions.erase(lifeCounter->directions.begin() + this->lives);
                    lifeCounter->rotations.erase(lifeCounter->rotations.begin() + this->lives);
                    lifeCounter->scaling.erase(lifeCounter->scaling.begin() + this->lives);
                    lifeCounter->UpdateNumInstance();
                    pacman->SetInvulnerable(true);
                    if (this->lives == 0) {
                        this->state = GAME_DEFEAT;
                    }
                    else {
                        ResetPlayerAndGhosts();
                    }
                }
            }
        }

    }

}

void Game::ResetPlayerAndGhosts() {
    pacman->gameObjects[pacman->GetCurrentModelIndex()]->positions[0]  = glm::vec3(7.5f, 0.0f, 13.5f);
    pacman->gameObjects[pacman->GetCurrentModelIndex()]->directions[0] = glm::vec3(0.0f, 0.0f, -1.0f);
    pacman->UpdateOtherGameObjects();

    if (blinky->IsAlive()) {
        blinky->gameObject->positions[0]  = glm::vec3(19.0f, 0.0f, 13.75f);
        blinky->gameObject->directions[0] = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    if (clyde->IsAlive()) {
        clyde->gameObject->positions[0]  = glm::vec3(16.0f, 0.0f, 15.5f);
        clyde->gameObject->directions[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    if (inky->IsAlive()) {
        inky->gameObject->positions[0]  = glm::vec3(16.0f, 0.0f, 12.25f);
        inky->gameObject->directions[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    if (pinky->IsAlive()) {
        pinky->gameObject->positions[0]  = glm::vec3(16.0f, 0.0f, 13.85f);
        pinky->gameObject->directions[0] = glm::vec3(-1.0f, 0.0f, 0.0f);
    }
}
