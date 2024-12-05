#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <deque>

#include "Ghost.hpp"
#include "Blinky.hpp"
#include "Clyde.hpp"
#include "Inky.hpp"
#include "Pinky.hpp"
#include "GameObjectBase.hpp"

struct GhostMapping {
    int blinkyIndex;
    int clydeIndex;
    int inkyIndex;
    int pinkyIndex;
};

class VulnerableGhost final : public Ghost {

public:
    std::vector<GameObjectBase*> gameObjectsBlue;
    std::vector<GameObjectBase*> gameObjectsWhite;

    GhostMapping ghostMapping = { .blinkyIndex= 0, .clydeIndex= 1, .inkyIndex= 2, .pinkyIndex= 3};

    explicit VulnerableGhost(Blinky* blinky, Clyde* clyde, Inky* inky, Pinky* pinky, std::pair<size_t, size_t> levelMatrixDim);
	~VulnerableGhost() override;

    [[nodiscard]] int GetCurrentModelIndex() const;
    [[nodiscard]] GameObjectBase* GetCurrentGameObject() const;
    [[nodiscard]] bool IsActive() const;
    void SetActive(bool active);
    void UpdateOtherGameObjects(size_t instanceIndex) const;
	void Move(double deltaTime, GameObjectBase* mazeWall) override;
	void Draw(double deltaTime) override;
    void RemoveAnInstace(size_t instanceIndex);
    void AddAnInstance(const glm::vec3& position, const glm::vec3& direction, float rotation, const glm::vec3& scale);

private:

    const float  VULNERABLE_GHOST_SPEED = 5.0f;
    const float  MAX_DT = 0.016f;                        // Equivalent to a frame rate of around 60 FPS
    const float  CHANGE_DIRECTION_PROBABILITY = 0.60f;   // Probability of change once minimum time is exceeded
    const size_t MAX_RECENT_DIRECTIONS = 10;             // Maximum number of directions to remember
    const float  CHANGE_DIRECTION_TIME_LIMIT = 2.0f;     // Minimum limit of 2 seconds before allowing a change
    const double MODEL_SWAP_TIME_LIMIT = 0.04;           // Maximum limit of 0.04 seconds therefore the model will be changed
    const double ACTIVATION_TIME_LIMIT = 6.0f;           // Maximum limit of 6 seconds therefore the drawing will be deactivated
    const float  ALTERNATION_INTERVAL = 0.5f;            // Time between alternations in seconds
    const int    MAX_ALTERNATION = 5;                    // Maximum number of alternations

    Blinky* blinky;
    Clyde*  clyde;
    Inky*   inky;
	Pinky*  pinky;
    std::deque<glm::vec3> recentDirections;
    std::pair<size_t, size_t> levelMatrixDim;
    int    currentModelIndex = 0;
    int    alternateCount = 0;
    bool   isActive;
    bool   drawBlue;
    float  changeDirectionTimeAccumulator = 0.0f;
    double alternationTimeAccumulator = 0.0f;
    double modelSwapTimeAccumulator = 0.0f;
    double activationTimeAccumulator = 0.0f;
	
    [[nodiscard]] bool doCollisions(const GameObjectBase* mazeWall, size_t instanceIndex) const;
    [[nodiscard]] int countDirectionFrequency(const glm::vec3& direction) const;
    void init() override;
    void updateRecentDirections(const glm::vec3& chosenDirection);
	void checkIfTeleportIsNeeded(size_t instanceIndex) const;
    void syncGhosts(bool syncThis) const;
    void syncAlternationGhosts() const;
    void moveInstance(double deltaTime, GameObjectBase* mazeWall, size_t instanceIndex);
};