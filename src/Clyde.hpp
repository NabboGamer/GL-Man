#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <deque>
#include <algorithm>

#include "Ghost.hpp"
#include "GameObjectBase.hpp"

class Clyde : public Ghost {

public:
	GameObjectBase* gameObject;

    Clyde();
	~Clyde();

	void Move(double deltaTime, GameObjectBase* mazeWall) override;
	void Draw(double deltaTime) override;

private:
    const float CLYDE_SPEED = 4.0f;
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
    bool skipFirstMovement = false;

	void init() override;
	bool doCollisions(GameObjectBase* mazeWall);
    int  countDirectionFrequency(const glm::vec3& direction) const;
    void updateRecentDirections(const glm::vec3& chosenDirection);

};