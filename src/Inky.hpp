#pragma once

#include <glm/glm.hpp>
#include <deque>

#include "Ghost.hpp"
#include "GameObjectBase.hpp"

class Inky final : public Ghost {

public:
	GameObjectBase* gameObject;

	explicit Inky(std::pair<size_t, size_t> levelMatrixDim);
	~Inky() override;

    [[nodiscard]] bool IsAlive() const;
    void SetAlive(bool alive);

	void Move(double deltaTime, GameObjectBase* mazeWall) override;
	void Draw(double deltaTime) override;

private:
    const float  INKY_SPEED = 3.0f;
    const float  MAX_DT = 0.016f;                       // Equivalent to a frame rate of around 60 FPS
    const float  MIN_TIME_BEFORE_CHANGE = 2.0f;         // Minimum time in seconds before allowing a change
    const float  CHANGE_DIRECTION_PROBABILITY = 0.60f;  // Probability of change once minimum time is exceeded
    const size_t MAX_RECENT_DIRECTIONS = 10;            // Maximum number of directions to remember
    
    float timeSinceLastChange = 0.0f;                   // Time elapsed since last change
    std::deque<glm::vec3> recentDirections;             // Queue of the last chosen directions
    std::pair<size_t, size_t> levelMatrixDim;
    bool skipFirstMovement = false;
    bool isAlive;

	
    [[nodiscard]] bool doCollisions(const GameObjectBase* mazeWall) const;
    [[nodiscard]] int  countDirectionFrequency(const glm::vec3& direction) const;
    void init() override;
    void updateRecentDirections(const glm::vec3& chosenDirection);
    void checkIfTeleportIsNeeded(float speed) const;

};