#pragma once

#include <vector>
#include <tuple>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GameLevel.hpp"
#include "custom_types.hpp"
//#include "power_up.h"

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
    GAME_DEFEAT
};

struct PermittedDirections {
    bool DIRECTION_UP    = true;
    bool DIRECTION_DOWN  = true;
    bool DIRECTION_RIGHT = true;
    bool DIRECTION_LEFT  = true;
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {

public:
    // game state
    GameState                state;
    bool                     keys[1024];
    bool                     keysProcessed[1024];
    unsigned int             width, height;
    PermittedDirections      permittedDirections;
    std::vector<GameLevel*>  Levels;
    unsigned int             level;
    unsigned int             lives;
    unsigned int             score;

    // constructor/destructor
    Game(unsigned int width, unsigned int height, CustomStructs::Config& config);
    ~Game();

    // initialize game state (load all shaders/textures/levels)
    void Init();
    [[nodiscard]] bool ContinueInit() const;

    // game loop
    void ProcessInput(double dt);
    void Update(double dt);
    void Render(double dt) const;
    void DoCollisions(double dt);
    
    // reset
    //void ResetLevel();
    static void ResetPlayerAndGhosts();

private:
    const double CHOMP_INTERVAL = 0.5;
    const double SPAWN_PROTECTION_TIME_LIMIT = 3.0f;

    glm::vec3 cameraPos;		                        // Camera position
    glm::vec3 cameraAt;			                        // Point where the camera "looks"
    glm::vec3 up;				                        // Up vector
    glm::vec3 cameraDir;		                        // Direction of gaze (the direction is reversed)
    glm::vec3 cameraSide;		                        // Direction of the lateral movement 
    glm::vec3 cameraUp;			                        // Vector that completes the orthonormal basis of the camera
	double    chompTimer = 0.0;                         // Timer utilized to decide if reproduce or not the pacman chomp sound
    int       ghostCounter = 0;                         // Counts the number of ghosts eaten during the effect of an energizer
    double    spawnProctectionTimeAccumulator = 0.0f;
    CustomStructs::Config& config;
	
};