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

namespace {
    constexpr float PLAYER_SPEED = 7.5f;
    float           scaleX;
    float           scaleY;
    float           scaleText;
    int             updating = 0;

    struct SharedParams {
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec3 lightDir;
        glm::vec3 lightPos;
        glm::vec3 lightSpecular;
        float materialShininess;
    };

    struct OtherParams {
        glm::vec3 lightAmbient;
        glm::vec3 lightDiffuse;
    };
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

Game::Game(const unsigned int width, const unsigned int height, CustomStructs::Config& config)
    : state(GAME_ACTIVE), keys(), keysProcessed(), width(width),
      height(height), level(0), lives(3), score(0), cameraPos(),
      cameraAt(), up(),cameraDir(),cameraSide(),cameraUp(), config(config) { }

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
    ResourceManager::LoadShader("./shaders/hdr.vs",        "./shaders/hdr.fs",        nullptr, "hdrShader");
    ResourceManager::LoadShader("./shaders/stencil.vs",    "./shaders/stencil.fs",    nullptr, "stencilShader");
    /*ResourceManager::LoadShader("particle.vs", "particle.fs", nullptr, "particle");*/

    /// Configure Shaders
    // Insert uniform variable in vertex shader(only global variables, i.e. the same for all shaders)
    this->cameraPos  = glm::vec3( -17.0, 22.5, 15.0);
    this->cameraAt   = glm::vec3(  10.0,  1.0, 15.0);
    this->up         = glm::vec3(   0.0,  1.0,  0.0);
    this->cameraDir  = glm::normalize(cameraPos - cameraAt);
    this->cameraSide = glm::normalize(glm::cross(up, cameraDir));
    this->cameraUp = glm::normalize(glm::cross(cameraDir, cameraSide));
    glm::mat4 view = glm::lookAt(cameraPos, cameraAt, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(35.0f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 55.0f);
    const glm::vec3 lightDir = glm::normalize(cameraAt - glm::vec3(-17.0, 27.0, 15.0));

    SharedParams shared;
    shared.projection = projection;
    shared.view = view;
    shared.lightDir = lightDir;
    shared.lightPos = glm::vec3(-17.0, 27.0, 15.0);
    shared.lightSpecular = glm::vec3(0.2f, 0.2f, 0.2f);
    shared.materialShininess = 32.0f;

    OtherParams other1, other2;
    other1.lightAmbient = glm::vec3(0.7f, 0.7f, 0.7f);
    other1.lightDiffuse = glm::vec3(0.9f, 0.9f, 0.9f);

    other2.lightAmbient = glm::vec3(0.07f, 0.07f, 0.07f);
    other2.lightDiffuse = glm::vec3(0.7f, 0.7f, 0.7f);

    unsigned int uboShared, uboOther1, uboOther2;
    glGenBuffers(1, &uboShared);
    glBindBuffer(GL_UNIFORM_BUFFER, uboShared);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + 3 * (sizeof(glm::vec3) + 4) + sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &uboOther1);
    glBindBuffer(GL_UNIFORM_BUFFER, uboOther1);
    glBufferData(GL_UNIFORM_BUFFER, 2 * (sizeof(glm::vec3) + 4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &uboOther2);
    glBindBuffer(GL_UNIFORM_BUFFER, uboOther2);
    glBufferData(GL_UNIFORM_BUFFER, 2 * (sizeof(glm::vec3) + 4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformBlockBinding(ResourceManager::GetShader("mazeWallShader").id,    glGetUniformBlockIndex(ResourceManager::GetShader("mazeWallShader").id,    "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("mazeFloorShader").id,   glGetUniformBlockIndex(ResourceManager::GetShader("mazeFloorShader").id,   "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("dotShader").id,         glGetUniformBlockIndex(ResourceManager::GetShader("dotShader").id,         "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("energizerShader").id,   glGetUniformBlockIndex(ResourceManager::GetShader("energizerShader").id,   "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("pacmanShader").id,      glGetUniformBlockIndex(ResourceManager::GetShader("pacmanShader").id,      "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("ghostShader").id,       glGetUniformBlockIndex(ResourceManager::GetShader("ghostShader").id,       "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("bonusSymbolShader").id, glGetUniformBlockIndex(ResourceManager::GetShader("bonusSymbolShader").id, "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("lifeCounterShader").id, glGetUniformBlockIndex(ResourceManager::GetShader("lifeCounterShader").id, "Shared"), 0);
    glUniformBlockBinding(ResourceManager::GetShader("mazeWallShader").id,    glGetUniformBlockIndex(ResourceManager::GetShader("mazeWallShader").id,    "Other"), 1);
    glUniformBlockBinding(ResourceManager::GetShader("mazeFloorShader").id,   glGetUniformBlockIndex(ResourceManager::GetShader("mazeFloorShader").id,   "Other"), 1);
    glUniformBlockBinding(ResourceManager::GetShader("pacmanShader").id,      glGetUniformBlockIndex(ResourceManager::GetShader("pacmanShader").id,      "Other"), 1);
    glUniformBlockBinding(ResourceManager::GetShader("ghostShader").id,       glGetUniformBlockIndex(ResourceManager::GetShader("ghostShader").id,       "Other"), 1);
    glUniformBlockBinding(ResourceManager::GetShader("bonusSymbolShader").id, glGetUniformBlockIndex(ResourceManager::GetShader("bonusSymbolShader").id, "Other"), 1);
    glUniformBlockBinding(ResourceManager::GetShader("lifeCounterShader").id, glGetUniformBlockIndex(ResourceManager::GetShader("lifeCounterShader").id, "Other"), 1);
    glUniformBlockBinding(ResourceManager::GetShader("dotShader").id,         glGetUniformBlockIndex(ResourceManager::GetShader("dotShader").id,         "Other"), 2);
    glUniformBlockBinding(ResourceManager::GetShader("energizerShader").id,   glGetUniformBlockIndex(ResourceManager::GetShader("energizerShader").id,   "Other"), 2);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboShared);
    glBindBuffer(GL_UNIFORM_BUFFER, uboShared);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,                                               sizeof(glm::mat4),   &shared.projection);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4),                               sizeof(glm::mat4),   &shared.view);
    glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4),                             sizeof(glm::vec3)+4, &shared.lightDir);
    glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4) + sizeof(glm::vec3)+4,       sizeof(glm::vec3)+4, &shared.lightPos);
    glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4) + 2*(sizeof(glm::vec3)+4),   sizeof(glm::vec3)+4, &shared.lightSpecular);
    glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4) + 3*(sizeof(glm::vec3)+4),   sizeof(float),       &shared.materialShininess);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboOther1);
    glBindBuffer(GL_UNIFORM_BUFFER, uboOther1);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,                     sizeof(glm::vec3)+4, &other1.lightAmbient);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec3)+4,   sizeof(glm::vec3)+4, &other1.lightDiffuse);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboOther2);
    glBindBuffer(GL_UNIFORM_BUFFER, uboOther2);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,                     sizeof(glm::vec3)+4, &other2.lightAmbient);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec3)+4,   sizeof(glm::vec3)+4, &other2.lightDiffuse);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    ResourceManager::GetShader("hdrShader").Use().SetMatrix4("projection", projection);

    ResourceManager::GetShader("stencilShader").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("stencilShader").Use().SetMatrix4("view", view);
    ResourceManager::GetShader("stencilShader").Use().SetVector3f("color", glm::vec3(0.988f, 0.812f, 0.0f));

    /// Load Textures
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/wall_diffuse_360.png").c_str(), "mazeWallDiffuseTexture", true);
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/wall_specular_360.png").c_str(), "mazeWallSpecularTexture", false);
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/floor_diffuse_360.png").c_str(), "mazeFloorDiffuseTexture", true);
    ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/floor_specular_360.png").c_str(), "mazeFloorSpecularTexture", false);

    /// Load Models
    ResourceManager::LoadModel("../res/objects/powerup/coin/coin.obj", "dotModel", false);
    ResourceManager::LoadModel("../res/objects/powerup/coin/coin.obj", "energizerModel", false);
    ResourceManager::LoadModel("../res/objects/powerup/cherries/cherries.obj", "cherriesModel", true);
    ResourceManager::LoadModel("../res/objects/powerup/cherries/cherries.obj", "cherriesModelStencil", false);
    ResourceManager::LoadModel("../res/objects/powerup/cherries/cherries.obj", "cherriesFruitCounterModel", true);
    ResourceManager::LoadModel("../res/objects/pacman/pacman7/pacman7.obj", "lifeCounterPacmanModel", true);

    /// Load Levels
    const auto levelOne = new GameLevel();
    levelOne->Load(FileSystem::getPath("../res/levels/one.lvl").c_str());
    this->Levels.push_back(levelOne);
    this->level = 0;
}

bool Game::ContinueInit() const {
    /// Configure Game Objects
    const auto levelMatrixDim = this->Levels[this->level]->levelMatrixDim;
    if (updating == 0) {
        pacman = new PacMan();
        updating += 1;
        return false;
    }
    else if (updating == 1) {
        blinky = new Blinky(levelMatrixDim);
        updating += 1;
        return false;
    }
    else if (updating == 2) {
        clyde = new Clyde(levelMatrixDim);
        updating += 1;
        return false;
    }
    else if (updating == 3) {
        inky = new Inky(levelMatrixDim);
        updating += 1;
        return false;
    }
    else if (updating == 4) {
        pinky = new Pinky(levelMatrixDim);
        updating += 1;
        return false;
    }
    else if (updating == 5) {
        vulnerableGhost = new VulnerableGhost(blinky, clyde, inky, pinky, levelMatrixDim);
        updating += 1;
        return false;
    }
    else if (updating == 6) {
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
        pacmanChompSound        ->setDefaultVolume(0.8f);
        ghostNormalMoveSound    ->setDefaultVolume(0.8f);
        pacmanEatFruitSound     ->setDefaultVolume(1.0f);
        ghostTurnBlueSound      ->setDefaultVolume(0.8f);
        pacmanEatGhostSound     ->setDefaultVolume(1.0f);
        pacmanDeathSound        ->setDefaultVolume(1.0f);
        pacmanEatsAllGhostsSound->setDefaultVolume(1.0f);
        victorySound            ->setDefaultVolume(1.0f);

        /// Configure render-specific objects
        text = new TextRenderer(this->width, this->height);
        text->Load(FileSystem::getPath("../res/fonts/eight_bit_dragon.ttf"), 32);
        postProcessor = new PostProcessor(this->width, this->height, true, 4,
                                          &ResourceManager::GetShader("hdrShader"), false,
                                          0.5f, 2.2f);

        scaleX = static_cast<float>(this->width)  / 2048.0f;
        scaleY = static_cast<float>(this->height) / 1152.0f;
        scaleText = std::min(scaleX, scaleY);
       
        updating += 1;
        return false;
    }
    else {
        return true;
    }
       
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

        /*if (this->keys[GLFW_KEY_Q]) {
            if (postProcessor->GetExposure() > 0.0f) {
                postProcessor->SetExposure(postProcessor->GetExposure() - 0.001f);
            }
            else {
                postProcessor->SetExposure(0.0f);
            }
            //std::cout << "Exposure: "<< exposure << std::endl;

        }
        else if (this->keys[GLFW_KEY_E]) {
            postProcessor->SetExposure(postProcessor->GetExposure() + 0.001f);
            //std::cout << "Exposure: " << exposure << std::endl;
        }*/
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilMask(0x00);
    if (this->state == GAME_ACTIVE || this->state == GAME_WIN || this->state == GAME_DEFEAT) {
        // begin rendering to postprocessing framebuffer
        if (!postProcessor->IsInitialized()) {
            postProcessor->SetInitialized(true, this->config);
        }
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

        // render text (don't include in postprocessing)
        const float widthFloat = static_cast<float>(this->width);
        const std::string scoreString = std::to_string(this->score);
        text->RenderText("1UP",        (widthFloat / 2.0f) - (widthFloat /  5.0f), 10*scaleY, 1.0f*scaleText);
        text->RenderText(scoreString,    (widthFloat / 2.0f) - (widthFloat /  5.0f), 50*scaleY, 1.0f*scaleText);
        text->RenderText("HIGH SCORE", (widthFloat / 2.0f) - (widthFloat / 20.0f), 10*scaleY, 1.0f*scaleText);
        text->RenderText(scoreString,    (widthFloat / 2.0f) - (widthFloat / 20.0f), 50*scaleY, 1.0f*scaleText);
        /*std::string exposure = "exposure=" + std::to_string(postProcessor->GetExposure());
        text->RenderText(exposure,    (widthFloat / 2.0f) - (widthFloat / 20.0f), 80, 1.0f);*/
    }
    if (this->state == GAME_WIN) {
        const float widthFloat = static_cast<float>(this->width);
        const float heightFloat = static_cast<float>(this->height);
        text->RenderText("You WON!!!",((widthFloat  / 2.0f) - (widthFloat  / 20.0f)),        
                                        ((heightFloat / 2.0f) - (heightFloat /  9.0f)),
										1.0f*scaleText,
										glm::vec3(0.0f, 1.0f, 0.0f));

        text->RenderText("Press ESC to quit",((widthFloat  / 2.0f) - (widthFloat  / 20.0f) - 70.0f*scaleX),
                                               ((heightFloat / 2.0f) - (heightFloat /  9.0f) + 40.0f*scaleY),
                                               1.0f*scaleText, 
                                               glm::vec3(0.0f, 1.0f, 0.0f));
        stopCurrentSound();
    }
    if (this->state == GAME_DEFEAT) {
        const float widthFloat = static_cast<float>(this->width);
        const float heightFloat = static_cast<float>(this->height);
        text->RenderText("You LOST!!!",((widthFloat  / 2.0f) - (widthFloat  / 20.0f)),
                                         ((heightFloat / 2.0f) - (heightFloat /  9.0f)),
                                         1.0f*scaleText,
                                         glm::vec3(1.0f, 0.0f, 0.0f));

        text->RenderText("Press ESC to quit",((widthFloat  / 2.0f) - (widthFloat  / 20.0f) - 60.0f*scaleX),
                                               ((heightFloat / 2.0f) - (heightFloat /  9.0f) + 40.0f*scaleY),
                                               1.0f*scaleText, 
                                               glm::vec3(1.0f, 0.0f, 0.0f));
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
