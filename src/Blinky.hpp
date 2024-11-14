#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <deque>

#include "Ghost.hpp"
#include "GameObjectBase.hpp"

class Blinky final : public Ghost {

public:
	GameObjectBase* gameObject;

	explicit Blinky(std::pair<size_t, size_t> levelMatrixDim);
	~Blinky() override;

	void Move(double deltaTime, GameObjectBase* mazeWall) override;
	void Draw(double deltaTime) override;

private:
    const float BLINKY_SPEED = 5.0f;
    const float MAX_DT = 0.016f;                       // Equivalent to a frame rate of around 60 FPS
    const float MIN_TIME_BEFORE_CHANGE = 2.0f;         // Minimum time in seconds before allowing a change
    const float CHANGE_DIRECTION_PROBABILITY = 0.60f;  // Probability of change once minimum time is exceeded
    const size_t MAX_RECENT_DIRECTIONS = 10;           // Maximum number of directions to remember
    // Vector of possible directions
    const std::vector<glm::vec3> possibleDirections = {
        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  0.0f, -1.0f)
    };
    float timeSinceLastChange = 0.0f;                  // Time elapsed since last change
    std::deque<glm::vec3> recentDirections;            // Queue of the last chosen directions
    std::pair<size_t, size_t> levelMatrixDim;
    

	void init() override;
	bool doCollisions(const GameObjectBase* mazeWall) const;
    int countDirectionFrequency(const glm::vec3& direction) const;
    void updateRecentDirections(const glm::vec3& chosenDirection);
	void checkIfTeleportIsNeeded(float speed) const;

};