#include <windows.h>
#include <random>
#include <algorithm>
#include <unordered_map>

#include "Pinky.hpp"
#include "custom_types.hpp"
#include "Utility.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"
#include "GameObjectFromModel.hpp"

Pinky::Pinky(const std::pair<size_t, size_t> levelMatrixDim) : Ghost(), levelMatrixDim(levelMatrixDim) {
	this->Pinky::init();
}

Pinky::~Pinky() {
	delete gameObject;
}

void Pinky::Move(double deltaTime, GameObjectBase* mazeWall) {
    // Avoid excessive movements due to possible framerate drops, as the movement depends on it
    float effectiveDt = std::min(static_cast<float>(deltaTime), MAX_DT);
    float speed = PINKY_SPEED * effectiveDt;

    // Check if Pinky is in the starting point
    glm::vec3 currentPos = this->gameObject->positions[0];
    if (currentPos.x >= 15.0f && currentPos.x <= 17.0f &&
        currentPos.z >= 12.0f && currentPos.z <= 16.0f) {
        // Posizione target
        auto targetPos = glm::vec3(19.0f, 0.0f, 13.75f);

        glm::vec3 direction = glm::normalize(targetPos - currentPos);
        this->gameObject->directions[0] = direction;

        glm::vec3 newPosition = currentPos + speed * direction;
        this->gameObject->positions[0] = newPosition;

        if (glm::distance(currentPos, targetPos) <= speed) {
            this->gameObject->positions[0] = targetPos;
            this->gameObject->directions[0] = glm::vec3(0.0f, 0.0f, -1.0f);
            this->timeSinceLastChange = 0.0f;
        }
        return;
    }

    glm::vec3 currentDirection = this->gameObject->directions[0];
    // Sanitize the direction vector (Avoid that there are components with negative zeros)
    glm::vec3 oppositeDirection = Utility::NormalizeZeros(-currentDirection);

    glm::vec3 newPosition = this->gameObject->positions[0] + speed * currentDirection;

    // Adds elapsed time to timer
    timeSinceLastChange += effectiveDt;

    // Random Number Generator
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0, 1);

    // Let's check if it is possible to change direction
    bool forceDirectionChange = false;
    if (timeSinceLastChange >= MIN_TIME_BEFORE_CHANGE) {
        // By multiplying by effectiveDt, which represents the time elapsed in the last frame, 
        // you are essentially normalizing the probability of change per unit of time. 
        // Thus, the probability of spontaneous direction change per second will remain constant, 
        // regardless of the duration of the single frame.
        forceDirectionChange = (dis(gen) < CHANGE_DIRECTION_PROBABILITY * effectiveDt);
    }

    this->gameObject->positions[0] = newPosition;
    bool doesItCollide = this->doCollisions(mazeWall);
    this->gameObject->positions[0] -= speed * currentDirection;

    // If there is no collision, and we do not force a change of direction, it continues in the current direction.
    if (!doesItCollide && !forceDirectionChange) {
        this->gameObject->positions[0] += speed * currentDirection;
        // Check for teleport
        this->checkIfTeleportIsNeeded(speed);
        return;
    }

    if (doesItCollide) {
        oppositeDirection = currentDirection;
    }
    else {
        oppositeDirection = glm::vec3(0.0f);
    }

    // Remove the opposite direction to the direction of travel
    std::vector<glm::vec3> validDirections;
    for (const auto& direction : possibleDirections) {
        if (direction != oppositeDirection) {
            validDirections.push_back(direction);
        }
    }

    // Remove directions that would lead to a collision
    std::vector<glm::vec3> safeDirections;
    for (const auto& direction : validDirections) {
        newPosition = this->gameObject->positions[0] + speed * direction;
        this->gameObject->positions[0] = newPosition;
        bool doesItCollide2 = this->doCollisions(mazeWall);
        this->gameObject->positions[0] -= speed * direction;
        if (!doesItCollide2) {
            safeDirections.push_back(direction);
        }
    }

    if (!safeDirections.empty()) {
        glm::vec3 chosenDirection;

        if (forceDirectionChange) {
            // If the change is spontaneous, we mainly use the less frequent direction
            // with a small probability of random choice
            static std::uniform_int_distribution<> dirDis(0, 99);
            // 90% of the time use minimum frequency logic
            if (dirDis(gen) < 90) { 

                std::unordered_map<glm::vec3, int, Utility::Vec3Hash> directionFrequency;
                for (const auto& direction : safeDirections) {
                    directionFrequency[direction] = countDirectionFrequency(direction);
                }

                auto minFreqIt = ranges::min_element(safeDirections,
                                                     [&directionFrequency](const glm::vec3& a, const glm::vec3& b) {
	                                                     return directionFrequency[a] < directionFrequency[b];
                                                     });

                chosenDirection = *minFreqIt;
            }
            else { // 10% of the time chooses randomly
                std::uniform_int_distribution<> randomDir(0, static_cast<int>(safeDirections.size()) - 1);
                chosenDirection = safeDirections[randomDir(gen)];
            }
            // Reset the timer when we change direction spontaneously
            timeSinceLastChange = 0.0f;
        }
        else {
            // Collision Behavior
            std::unordered_map<glm::vec3, int, Utility::Vec3Hash> directionFrequency;

            for (const auto& direction : safeDirections) {
                directionFrequency[direction] = countDirectionFrequency(direction);
            }

            auto minFreqIt = ranges::min_element(safeDirections,
                                                 [&directionFrequency](const glm::vec3& a, const glm::vec3& b) {
	                                                 return directionFrequency[a] < directionFrequency[b];
                                                 });

            chosenDirection = *minFreqIt;

            // Timer reset also for forced changes due to collision
            timeSinceLastChange = 0.0f;
        }

        // Finally set the new direction and update the position
        this->gameObject->directions[0] = chosenDirection;
        this->gameObject->positions[0] += speed * chosenDirection;
        // Check for teleport
        this->checkIfTeleportIsNeeded(speed);
        updateRecentDirections(chosenDirection);
    }
    else {
        LoggerManager::LogDebug("Pinky has no clear direction, and remains stationary...");
    }

}

void Pinky::Draw(double deltaTime) {
	this->gameObject->Draw();
}

void Pinky::init() {
	const std::vector<glm::vec3> pinkyPositions  = { glm::vec3(16.0f, 0.0f, 13.85f) };
	const std::vector<glm::vec3> pinkyDirections = { glm::vec3(-1.0f, 0.0f, 0.0f) };
	const std::vector<float>     pinkyRotations  = { 0.0f };
	const std::vector<glm::vec3> pinkyScaling    = { glm::vec3(0.25f) };

	ResourceManager::LoadModel("../res/objects/ghosts/pinky/pinky.obj", "pinkyModel");
	this->gameObject = new GameObjectFromModel(pinkyPositions,
											   pinkyDirections,
											   pinkyRotations,
											   pinkyScaling,
											   &ResourceManager::GetShader("ghostShader"),
											   &ResourceManager::GetModel("pinkyModel"));
}

bool Pinky::doCollisions(const GameObjectBase* mazeWall) const {
	const auto blinkyObb = this->gameObject->GetTransformedBoundingBox(0);

	// CHECK COLLISION PINKY-WALL
	const size_t numInstancesMazeWall = mazeWall->GetNumInstances();
	for (size_t i = 0; i < numInstancesMazeWall; i++) {
		auto mazeWallObb = mazeWall->GetTransformedBoundingBox(i);
		if (const bool collision = Pinky::checkCollision(blinkyObb, mazeWallObb)) {
            LoggerManager::LogDebug("There was a collision between PINKY and WALL number {}", i);
            // RESOLVE COLLISION PINKY-WALL
            const glm::vec3 correction = Pinky::resolveCollision(blinkyObb, mazeWallObb);
            this->gameObject->positions[0] += correction; // Apply the correction vector
            return true;
        }
	}
	return false;
}

// Counts the frequency of each direction in the recent queue
int Pinky::countDirectionFrequency(const glm::vec3& direction) const {
    return static_cast<int>(ranges::count(recentDirections, direction));
}

void Pinky::updateRecentDirections(const glm::vec3& chosenDirection) {
    if (recentDirections.size() >= MAX_RECENT_DIRECTIONS) {
        recentDirections.pop_front();
    }
    recentDirections.push_back(chosenDirection);
}

void Pinky::checkIfTeleportIsNeeded(float speed) const {
    const auto pinkyObb = this->gameObject->GetTransformedBoundingBox(0);
    const glm::vec3 pMin = pinkyObb.first;
    const glm::vec3 pMax = pinkyObb.second;
    const size_t columnDim = this->levelMatrixDim.second;

    if (pMax.z >= static_cast<float>(columnDim) && this->gameObject->directions[0] == glm::vec3(0.0f, 0.0f, 1.0f)) {
        this->gameObject->positions[0] = glm::vec3(this->gameObject->positions[0].x, this->gameObject->positions[0].y, 0.0f);
    }
    else if (pMin.z <= 0.0f && this->gameObject->directions[0] == glm::vec3(0.0f, 0.0f, -1.0f)) {
        this->gameObject->positions[0] = glm::vec3(this->gameObject->positions[0].x, this->gameObject->positions[0].y, static_cast<float>(columnDim) - 1.0f);
    }
}
