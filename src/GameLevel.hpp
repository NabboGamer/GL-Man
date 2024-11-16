#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObjectBase.hpp"


// GameLevel holds all GameObject as part of a GLMan level and 
// hosts functionality to load/render levels from the hard disk.
class GameLevel {

public:
    // level state
    std::vector<glm::vec3>  mazeWallPositions;
    GameObjectBase*         mazeWall;
    std::vector<glm::vec3>  mazeFloorPositions;
    GameObjectBase*         mazeFloor;
    std::vector<glm::vec3>  dotPositions;
    GameObjectBase*         dot;
    std::vector<glm::vec3>  energizerPositions;
    GameObjectBase*         energizer;

    std::pair<size_t, size_t> levelMatrixDim;

    // constructor
    GameLevel() { }
    ~GameLevel();

    // loads level from file
    void Load(const char *file);

    // render level
    void Draw() const;

    // check if the level is completed (if all the dots have been eaten)
    //bool IsCompleted();

private:
    // initialize level from tile data
    void init(std::vector<std::vector<unsigned int>> wallData);

};
