#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObjectBase.hpp"


// GameLevel holds all GameObject as part of a GLMan level and 
// hosts functionality to load/render levels from the hard disk.
class GameLevel {

public:
    const double FIRST_ACTIVATION_TIME_LIMIT = 10.0f;
    const double SECOND_ACTIVATION_TIME_LIMIT = 9.0f;

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

    GameObjectBase*         bonusSymbol;
    GameObjectBase*         bonusSymbolStencil;
    GameObjectBase*         fruitCounter;
    GameObjectBase*         lifeCounter;

    // constructor
    GameLevel() { }
    ~GameLevel();

    // loads level from file
    void Load(const char *file);

    // render level
    void Draw(double deltaTime);

    // Getter and Setters
    [[nodiscard]] bool   GetPlayerTakeBonusSymbol() const;
    [[nodiscard]] double GetFirstActivationTimeAccumulator() const;
    [[nodiscard]] double GetSecondActivationTimeAccumulator() const;
    [[nodiscard]] int    GetSymbolActive() const;

    void SetPlayerTakeBonusSymbol(bool value);
    void SetFirstActivationTimeAccumulator(double value);
    void SetSecondActivationTimeAccumulator(double value);
    void SetSymbolActive(int value);
    void SetBonusSymbolPosition(glm::vec3 newBonusSymbolPosition) const;


    // check if the level is completed (if all the dots have been eaten)
    [[nodiscard]] bool IsCompleted() const;

private:
    const float VISIBLE_DURATION = 0.7f;
    const float INVISIBLE_DURATION = 0.3f;

    double firstActivationTimeAccumulator = 0.0f;
    double secondActivationTimeAccumulator = 0.0f;
    bool   playerTakeBonusSymbol = false;
    int    symbolActive = 0;
    bool   energizerVisible = true;
    double visibilityTimer = 0.0f;

    // initialize level from tile data
    void init(std::vector<std::vector<unsigned int>> wallData);

    [[nodiscard]] bool shouldSpawnBonusSymbol(double deltaTime);

};
