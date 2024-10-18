#pragma once

#include <vector>
#include <tuple>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include "game_level.h"
//#include "power_up.h"

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Defines a Collision typedef that represents collision data
//typedef std::tuple<bool, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {

public:
    // game state
    GameState               state;
    bool                    keys[1024];
    bool                    keysProcessed[1024];
    unsigned int            width, height;

    //std::vector<GameLevel>  Levels;
    //std::vector<PowerUp>    PowerUps;
    //unsigned int            level;
    //unsigned int            lives;

    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();

    // initialize game state (load all shaders/textures/levels)
    void Init();

    // game loop
    /*void ProcessInput(float dt);
    void Update(float dt);*/
    void Render();
    //void DoCollisions();
    
    // reset
    //void ResetLevel();
    //void ResetPlayer();
    
    // powerups
    //void SpawnPowerUps(GameObject& block);
    //void UpdatePowerUps(float dt);

private:
    glm::vec3 cameraPos;		// Camera position
    glm::vec3 cameraAt;			// Point where the camera "looks"
    glm::vec3 up;				// Up vector
    glm::vec3 cameraDir;		// Direction of gaze (the direction is reversed)
    glm::vec3 cameraSide;		// Direction of the lateral movement 
    glm::vec3 cameraUp;			// Vector that completes the orthonormal basis of the camera

};