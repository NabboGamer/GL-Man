#include <windows.h>
#include <random>
#include <algorithm>
#include <unordered_map>

#include "VulnerableGhost.hpp"
#include "custom_types.hpp"
#include "Utility.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"
#include "GameObjectFromModel.hpp"
#include "Utility.hpp"


VulnerableGhost::VulnerableGhost(Blinky* blinky, Clyde* clyde, Inky* inky, Pinky* pinky, const std::pair<size_t, size_t> levelMatrixDim)
			   : Ghost(), blinky(blinky), clyde(clyde), inky(inky), pinky(pinky), levelMatrixDim(levelMatrixDim), isActive(false), drawBlue(true) {
    this->VulnerableGhost::init();
}

VulnerableGhost::~VulnerableGhost() {
    for (const auto gameObjectBasePtr : this->gameObjectsBlue) {
        delete gameObjectBasePtr;
    }
}

int VulnerableGhost::GetCurrentModelIndex() const {
    return this->currentModelIndex;
}

bool VulnerableGhost::IsActive() const {
    return this->isActive;
}

void VulnerableGhost::SetActive(const bool active) {
    this->isActive = active;
    if (active) {
        this->activationTimeAccumulator = 0.0f; // Reset the timer
        this->alternationTimeAccumulator = 0.0f;
        this->modelSwapTimeAccumulator = 0.0f;
        this->alternateCount = 0;
        this->drawBlue = true;
        this->syncGhosts(true);
    } else {
        this->syncGhosts(false);
    }
}

void VulnerableGhost::UpdateOtherGameObjects() const {
    for (int i = 0; i < 10; i++) {
        if (i != this->currentModelIndex) {
            this->gameObjectsBlue[i]->positions[0] = this->gameObjectsBlue[this->currentModelIndex]->positions[0];
            this->gameObjectsBlue[i]->directions[0] = this->gameObjectsBlue[this->currentModelIndex]->directions[0];
            this->gameObjectsWhite[i]->positions[0] = this->gameObjectsWhite[this->currentModelIndex]->positions[0];
            this->gameObjectsWhite[i]->directions[0] = this->gameObjectsWhite[this->currentModelIndex]->directions[0];
            /*this->gameObjectsBlue[i]->rotations[0]  = this->gameObjectsBlue[currentModelIndex]->rotations[0];
            this->gameObjectsBlue[i]->scaling[0]    = this->gameObjectsBlue[currentModelIndex]->scaling[0];*/
        }
    }
}

void VulnerableGhost::Move(double deltaTime, GameObjectBase* mazeWall) {
    auto currentGameObject = this->drawBlue ? this->gameObjectsBlue[this->currentModelIndex] : this->gameObjectsWhite[this->currentModelIndex];
    // Avoid excessive movements due to possible framerate drops, as the movement depends on it
    float effectiveDt = std::min(static_cast<float>(deltaTime), MAX_DT);
    float speed = this->VULNERABLE_GHOST_SPEED * effectiveDt;

    glm::vec3 currentDirection = currentGameObject->directions[0];
    // Sanitize the direction vector (Avoid that there are components with negative zeros)
    glm::vec3 oppositeDirection = Utility::NormalizeZeros(-currentDirection);

    glm::vec3 newPosition = currentGameObject->positions[0] + speed * currentDirection;

    // Adds elapsed time to timer
    this->changeDirectionTimeAccumulator += effectiveDt;

    // Random Number Generator
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0, 1);

    // Let's check if it is possible to change direction
    bool forceDirectionChange = false;
    if (this->changeDirectionTimeAccumulator >= this->CHANGE_DIRECTION_TIME_LIMIT) {
        // By multiplying by effectiveDt, which represents the time elapsed in the last frame, 
        // you are essentially normalizing the probability of change per unit of time. 
        // Thus, the probability of spontaneous direction change per second will remain constant, 
        // regardless of the duration of the single frame.
        forceDirectionChange = (dis(gen) < this->CHANGE_DIRECTION_PROBABILITY * effectiveDt);
    }

    currentGameObject->positions[0] = newPosition;
    bool doesItCollide = this->doCollisions(mazeWall);
    currentGameObject->positions[0] -= speed * currentDirection;

    // If there is no collision, and we do not force a change of direction, it continues in the current direction.
    if (!doesItCollide && !forceDirectionChange) {
        currentGameObject->positions[0] += speed * currentDirection;
        // Check for teleport
        this->checkIfTeleportIsNeeded(speed);
        this->UpdateOtherGameObjects();
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
    for (auto& direction : Utility::possibleDirections) {
        if (direction != oppositeDirection) {
            validDirections.push_back(direction);
        }
    }

    // Remove directions that would lead to a collision
    std::vector<glm::vec3> safeDirections;
    for (const auto& direction : validDirections) {
        newPosition = currentGameObject->positions[0] + speed * direction;
        currentGameObject->positions[0] = newPosition;
        bool doesItCollide2 = this->doCollisions(mazeWall);
        currentGameObject->positions[0] -= speed * direction;
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
            changeDirectionTimeAccumulator = 0.0f;
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
            changeDirectionTimeAccumulator = 0.0f;
        }

        // Finally set the new direction and update the position
        currentGameObject->directions[0] = chosenDirection;
        currentGameObject->positions[0] += speed * chosenDirection;
        // Check for teleport
        this->checkIfTeleportIsNeeded(speed);
        this->UpdateOtherGameObjects();
        this->updateRecentDirections(chosenDirection);
    }
    else {
        LoggerManager::LogDebug("VULNERABLE_GHOST has no clear direction, and remains stationary...");
    }

}

void VulnerableGhost::Draw(const double deltaTime) {

    if (this->isActive) {
        this->activationTimeAccumulator += deltaTime;

        // After 6 seconds, start the alternation process
        if (this->activationTimeAccumulator >= ACTIVATION_TIME_LIMIT) {
            // If the 5 alternations have occurred, deactivate the ghost
            if (this->alternateCount >= MAX_ALTERNATION) {
                this->SetActive(false);
                return;
            }

            // Accumulate time for alternation
            this->alternationTimeAccumulator += deltaTime;

            // Alternate every ALTERNATE_INTERVAL seconds
            if (this->alternationTimeAccumulator >= ALTERNATION_INTERVAL) {
                this->alternationTimeAccumulator -= ALTERNATION_INTERVAL;
                this->drawBlue = !this->drawBlue; // Switch between Blue and White
                // Increment alternateCount only after a full cycle (Blue -> White -> Blue)
                if (!this->drawBlue) { 
                    this->alternateCount++; // Increment only when returning to Blue
                }
            }
            this->syncAlternationGhosts();
        }
    }

    this->modelSwapTimeAccumulator += deltaTime;
    // When modelSwapTimeAccumulator is greater than or equal to MODEL_SWAP_TIME_LIMIT, 
    // it means that enough time (0.04 seconds) has passed for the model to change.
    if (this->modelSwapTimeAccumulator >= this->MODEL_SWAP_TIME_LIMIT) {
        // If the accumulated time is, for example, 0.06 seconds, we exceed MODEL_SWAP_TIME_LIMIT 
        // (0.04 seconds) and would have to change the model, but there would be 0.02 seconds left. 
        // This remaining time will be accumulated in the next loop, so we subtract it to "reset" 
        // modelSwapTimeAccumulator without losing the remaining time
        modelSwapTimeAccumulator -= MODEL_SWAP_TIME_LIMIT;
        // This is a formula to cyclically increase the model index, moving to the next one.
        // % 10 ensures that once the last model is reached, the index resets to 0. 
        // This causes the animation to loop.
        this->currentModelIndex = (this->currentModelIndex + 1) % 10;
    }
    GameObjectBase* currentGameObject = this->drawBlue ? this->gameObjectsBlue[this->currentModelIndex] : this->gameObjectsWhite[this->currentModelIndex];
    currentGameObject->Draw();
    this->UpdateOtherGameObjects();
}

void VulnerableGhost::init() {
	const std::vector<glm::vec3> vulnerableGhostPositions  = { glm::vec3(19.0f, 0.0f, 13.75f) };
	const std::vector<glm::vec3> vulnerableGhostDirections = { glm::vec3(0.0f, 0.0f, -1.0f) };
	const std::vector<float>     vulnerableGhostRotations  = { 0.0f };
	const std::vector<glm::vec3> vulnerableGhostScaling    = { glm::vec3(0.25f) };


    for (int i = 0; i < 10; i++) {
        std::string string = "vulnerable_ghost_blue";
        std::string completeString = string + std::to_string(i + 1);
        ResourceManager::LoadModel("../res/objects/ghosts/vulnerable_ghost_blue/" + completeString + "/" + completeString + ".obj", completeString);
        this->gameObjectsBlue.push_back(new GameObjectFromModel(vulnerableGhostPositions,
															    vulnerableGhostDirections,
															    vulnerableGhostRotations,
															    vulnerableGhostScaling,
                                                                &ResourceManager::GetShader("ghostShader"),
                                                                &ResourceManager::GetModel(completeString)));
    }

    for (int i = 0; i < 10; i++) {
        std::string string = "vulnerable_ghost_white";
        std::string completeString = string + std::to_string(i + 1);
        ResourceManager::LoadModel("../res/objects/ghosts/vulnerable_ghost_white/" + completeString + "/" + completeString + ".obj", completeString);
        this->gameObjectsWhite.push_back(new GameObjectFromModel(vulnerableGhostPositions,
                                                                 vulnerableGhostDirections,
                                                                 vulnerableGhostRotations,
                                                                 vulnerableGhostScaling,
                                                                 &ResourceManager::GetShader("ghostShader"),
                                                                 &ResourceManager::GetModel(completeString)));
    }

}
    
bool VulnerableGhost::doCollisions(const GameObjectBase* mazeWall) const {
    const auto currentGameObject = this->drawBlue ? this->gameObjectsBlue[this->currentModelIndex] : this->gameObjectsWhite[this->currentModelIndex];
	const auto vulnerableGhostObb = currentGameObject->GetTransformedBoundingBox(0);

	// CHECK COLLISION VULNERABLE_GHOST-WALL
	const size_t numInstancesMazeWall = mazeWall->GetNumInstances();
	for (size_t i = 0; i < numInstancesMazeWall; i++) {
		auto mazeWallObb = mazeWall->GetTransformedBoundingBox(i);
		if (checkCollision(vulnerableGhostObb, mazeWallObb)) {
            LoggerManager::LogDebug("There was a collision between VULNERABLE_GHOST and WALL number {}", i);
            // RESOLVE COLLISION VULNERABLE_GHOST-WALL
            const glm::vec3 correction = resolveCollision(vulnerableGhostObb, mazeWallObb);
            currentGameObject->positions[0] += correction; // Apply the correction vector
            return true;
        }
	}
	return false;
}

// Counts the frequency of each direction in the recent queue
int VulnerableGhost::countDirectionFrequency(const glm::vec3& direction) const {
    return static_cast<int>(ranges::count(recentDirections, direction));
}

void VulnerableGhost::updateRecentDirections(const glm::vec3& chosenDirection) {
    if (recentDirections.size() >= MAX_RECENT_DIRECTIONS) {
        recentDirections.pop_front();
    }
    recentDirections.push_back(chosenDirection);
}

void VulnerableGhost::checkIfTeleportIsNeeded(float speed) const {
    const auto currentGameObject = this->drawBlue ? this->gameObjectsBlue[this->currentModelIndex] : this->gameObjectsWhite[this->currentModelIndex];
    const auto blinkyObb = currentGameObject->GetTransformedBoundingBox(0);
    const glm::vec3 pMin = blinkyObb.first;
    const glm::vec3 pMax = blinkyObb.second;
    const size_t columnDim = this->levelMatrixDim.second;

    if (pMax.z >= static_cast<float>(columnDim) && currentGameObject->directions[0] == glm::vec3(0.0f, 0.0f, 1.0f)) {
        currentGameObject->positions[0] = glm::vec3(currentGameObject->positions[0].x, currentGameObject->positions[0].y, 0.0f);
    }
	else if (pMin.z <= 0.0f && currentGameObject->directions[0] == glm::vec3(0.0f, 0.0f, -1.0f)) {
        currentGameObject->positions[0] = glm::vec3(currentGameObject->positions[0].x, currentGameObject->positions[0].y, static_cast<float>(columnDim) - 1.0f);
    }
}

void VulnerableGhost::syncGhosts(const bool syncThis) const {
    if (syncThis) {
        if (drawBlue) {
            this->gameObjectsBlue[this->currentModelIndex]->positions[0] = blinky->gameObject->positions[0];
            this->gameObjectsBlue[this->currentModelIndex]->directions[0] = blinky->gameObject->directions[0];
            this->UpdateOtherGameObjects();
        } else {
            this->gameObjectsWhite[this->currentModelIndex]->positions[0] = blinky->gameObject->positions[0];
            this->gameObjectsWhite[this->currentModelIndex]->directions[0] = blinky->gameObject->directions[0];
            this->UpdateOtherGameObjects();
        }
        
    } else {
        if (drawBlue) {
            blinky->gameObject->positions[0] = this->gameObjectsBlue[this->currentModelIndex]->positions[0];
            blinky->gameObject->directions[0] = this->gameObjectsBlue[this->currentModelIndex]->directions[0];
        } else {
            blinky->gameObject->positions[0] = this->gameObjectsWhite[this->currentModelIndex]->positions[0];
            blinky->gameObject->directions[0] = this->gameObjectsWhite[this->currentModelIndex]->directions[0];
        }
    }
}

void VulnerableGhost::syncAlternationGhosts() const {
    if (drawBlue) {
        this->gameObjectsWhite[this->currentModelIndex]->positions[0] = this->gameObjectsBlue[this->currentModelIndex]->positions[0];
        this->gameObjectsWhite[this->currentModelIndex]->directions[0] = this->gameObjectsBlue[this->currentModelIndex]->directions[0];
        this->UpdateOtherGameObjects();
    }
    else {
        this->gameObjectsBlue[this->currentModelIndex]->positions[0] = this->gameObjectsWhite[this->currentModelIndex]->positions[0];
        this->gameObjectsBlue[this->currentModelIndex]->directions[0] = this->gameObjectsWhite[this->currentModelIndex]->directions[0];
        this->UpdateOtherGameObjects();
    }
}
