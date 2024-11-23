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


VulnerableGhost::VulnerableGhost(Blinky* blinky, Clyde* clyde, Inky* inky, Pinky* pinky, 
                                 const std::pair<size_t, size_t> levelMatrixDim)
			   : Ghost(), blinky(blinky), clyde(clyde), inky(inky), pinky(pinky),
                 levelMatrixDim(levelMatrixDim), isActive(false), drawBlue(true) {
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

GameObjectBase* VulnerableGhost::GetCurrentGameObject() const {
	return (this->drawBlue ? this->gameObjectsBlue[this->currentModelIndex] : this->gameObjectsWhite[this->currentModelIndex]);
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

void VulnerableGhost::UpdateOtherGameObjects(const size_t instanceIndex) const {
    for (int i = 0; i < 10; i++) {
        if (i != this->currentModelIndex) {
            this->gameObjectsBlue[i]->positions[instanceIndex]   = this->gameObjectsBlue[this->currentModelIndex]->positions[instanceIndex];
            this->gameObjectsBlue[i]->directions[instanceIndex]  = this->gameObjectsBlue[this->currentModelIndex]->directions[instanceIndex];
            this->gameObjectsWhite[i]->positions[instanceIndex]  = this->gameObjectsWhite[this->currentModelIndex]->positions[instanceIndex];
            this->gameObjectsWhite[i]->directions[instanceIndex] = this->gameObjectsWhite[this->currentModelIndex]->directions[instanceIndex];
        }
    }
}

void VulnerableGhost::Move(const double deltaTime, GameObjectBase* mazeWall) {
    const int numInstances = static_cast<int>(this->GetCurrentGameObject()->GetNumInstances());
    for (int j = 0; j < numInstances; ++j) {
        this->moveInstance(deltaTime, mazeWall, j);
    }
}

void VulnerableGhost::moveInstance(double deltaTime, GameObjectBase* mazeWall, size_t instanceIndex) {
    auto currentGameObject = this->GetCurrentGameObject();
    // Avoid excessive movements due to possible framerate drops, as the movement depends on it
    float effectiveDt = std::min(static_cast<float>(deltaTime), MAX_DT);
    float speed = this->VULNERABLE_GHOST_SPEED * effectiveDt;

    glm::vec3 currentDirection = currentGameObject->directions[instanceIndex];
    // Sanitize the direction vector (Avoid that there are components with negative zeros)
    glm::vec3 oppositeDirection = Utility::NormalizeZeros(-currentDirection);

    glm::vec3 newPosition = currentGameObject->positions[instanceIndex] + speed * currentDirection;

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

    currentGameObject->positions[instanceIndex] = newPosition;
    bool doesItCollide = this->doCollisions(mazeWall, instanceIndex);
    currentGameObject->positions[instanceIndex] -= speed * currentDirection;

    // If there is no collision, and we do not force a change of direction, it continues in the current direction.
    if (!doesItCollide && !forceDirectionChange) {
        currentGameObject->positions[instanceIndex] += speed * currentDirection;
        // Check for teleport
        this->checkIfTeleportIsNeeded(instanceIndex);
        this->UpdateOtherGameObjects(instanceIndex);
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
        newPosition = currentGameObject->positions[instanceIndex] + speed * direction;
        currentGameObject->positions[instanceIndex] = newPosition;
        bool doesItCollide2 = this->doCollisions(mazeWall, instanceIndex);
        currentGameObject->positions[instanceIndex] -= speed * direction;
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
        currentGameObject->directions[instanceIndex] = chosenDirection;
        currentGameObject->positions[instanceIndex] += speed * chosenDirection;
        // Check for teleport
        this->checkIfTeleportIsNeeded(instanceIndex);
        this->UpdateOtherGameObjects(instanceIndex);
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
    GameObjectBase* currentGameObject = this->GetCurrentGameObject();
    currentGameObject->Draw();
    const int numInstances = static_cast<int>(currentGameObject->GetNumInstances());
    for (int i = 0; i < numInstances; ++i) {
        this->UpdateOtherGameObjects(i);
    }
}

void VulnerableGhost::RemoveAnInstace(const size_t instanceIndex) {
    
    for (const auto gameObjectBlue : this->gameObjectsBlue) {
        gameObjectBlue->positions.erase(gameObjectBlue->positions.begin() + instanceIndex);
        gameObjectBlue->directions.erase(gameObjectBlue->directions.begin() + instanceIndex);
        gameObjectBlue->rotations.erase(gameObjectBlue->rotations.begin() + instanceIndex);
        gameObjectBlue->scaling.erase(gameObjectBlue->scaling.begin() + instanceIndex);
        gameObjectBlue->UpdateNumInstance();
    }

    for (const auto gameObjectWhite : this->gameObjectsWhite) {
        gameObjectWhite->positions.erase(gameObjectWhite->positions.begin() + instanceIndex);
        gameObjectWhite->directions.erase(gameObjectWhite->directions.begin() + instanceIndex);
        gameObjectWhite->rotations.erase(gameObjectWhite->rotations.begin() + instanceIndex);
        gameObjectWhite->scaling.erase(gameObjectWhite->scaling.begin() + instanceIndex);
        gameObjectWhite->UpdateNumInstance();
    }

    const int indexToRemove = static_cast<int>(instanceIndex);
    // Update the indexes in the map
    if (ghostMapping.blinkyIndex == indexToRemove) {
        this->ghostMapping.blinkyIndex = -1; //Indicates that it has been removed
    }
    else if (ghostMapping.clydeIndex == indexToRemove) {
        this->ghostMapping.clydeIndex = -1;
    }
    else if (ghostMapping.inkyIndex == indexToRemove) {
        this->ghostMapping.inkyIndex = -1;
    }
    else if (ghostMapping.pinkyIndex == indexToRemove) {
        this->ghostMapping.pinkyIndex = -1;
    }

    // Scale the indices for all subsequent ghosts
    if (ghostMapping.blinkyIndex > indexToRemove)  --ghostMapping.blinkyIndex;
    if (ghostMapping.clydeIndex  > indexToRemove)  --ghostMapping.clydeIndex;
    if (ghostMapping.inkyIndex   > indexToRemove)  --ghostMapping.inkyIndex;
    if (ghostMapping.pinkyIndex  > indexToRemove)  --ghostMapping.pinkyIndex;
    
}

void VulnerableGhost::AddAnInstance(const glm::vec3& position, const glm::vec3& direction, const float rotation, const glm::vec3& scale) {

    for (const auto gameObjectBlue : this->gameObjectsBlue) {
        gameObjectBlue->positions.push_back(position);
        gameObjectBlue->directions.push_back(direction);
        gameObjectBlue->rotations.push_back(rotation);
        gameObjectBlue->scaling.push_back(scale);
        gameObjectBlue->UpdateNumInstance();
    }

    for (const auto gameObjectWhite : this->gameObjectsWhite) {
        gameObjectWhite->positions.push_back(position);
        gameObjectWhite->directions.push_back(direction);
        gameObjectWhite->rotations.push_back(rotation);
        gameObjectWhite->scaling.push_back(scale);
        gameObjectWhite->UpdateNumInstance();
    }

    // Update mapping if necessary
    const int newIndex = static_cast<int>(this->gameObjectsBlue[0]->positions.size() - 1); // New Index of the instance

    // Find an unmapped ghost and assign it the new index
    if (ghostMapping.blinkyIndex == -1) {
        ghostMapping.blinkyIndex = newIndex;
    }
    else if (ghostMapping.clydeIndex == -1) {
        ghostMapping.clydeIndex = newIndex;
    }
    else if (ghostMapping.inkyIndex == -1) {
        ghostMapping.inkyIndex = newIndex;
    }
    else if (ghostMapping.pinkyIndex == -1) {
        ghostMapping.pinkyIndex = newIndex;
    }
}


void VulnerableGhost::init() {
	const std::vector<glm::vec3> vulnerableGhostPositions(4, glm::vec3(19.0f, 0.0f, 13.75f));
	const std::vector<glm::vec3> vulnerableGhostDirections(4, glm::vec3(0.0f, 0.0f, -1.0f));
	const std::vector<float>     vulnerableGhostRotations(4, 0.0f);
	const std::vector<glm::vec3> vulnerableGhostScaling(4, glm::vec3(0.25f));


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
    
bool VulnerableGhost::doCollisions(const GameObjectBase* mazeWall, size_t instanceIndex) const {
    const auto currentGameObject = this->GetCurrentGameObject();
	const auto vulnerableGhostObb = currentGameObject->GetTransformedBoundingBox(instanceIndex);

	// CHECK COLLISION VULNERABLE_GHOST-WALL
	const size_t numInstancesMazeWall = mazeWall->GetNumInstances();
	for (size_t i = 0; i < numInstancesMazeWall; i++) {
		auto mazeWallObb = mazeWall->GetTransformedBoundingBox(i);
		if (checkCollision(vulnerableGhostObb, mazeWallObb)) {
            LoggerManager::LogDebug("There was a collision between VULNERABLE_GHOST number {} and WALL number {}",instanceIndex,i);
            // RESOLVE COLLISION VULNERABLE_GHOST-WALL
            const glm::vec3 correction = resolveCollision(vulnerableGhostObb, mazeWallObb);
            currentGameObject->positions[instanceIndex] += correction; // Apply the correction vector
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

void VulnerableGhost::checkIfTeleportIsNeeded(const size_t instanceIndex) const {
    const auto currentGameObject = this->GetCurrentGameObject();
    const auto vulnerableGhostObb = currentGameObject->GetTransformedBoundingBox(instanceIndex);
    const glm::vec3 pMin = vulnerableGhostObb.first;
    const glm::vec3 pMax = vulnerableGhostObb.second;
    const size_t columnDim = this->levelMatrixDim.second;

    if (pMax.z >= static_cast<float>(columnDim) && currentGameObject->directions[instanceIndex] == glm::vec3(0.0f, 0.0f, 1.0f)) {
        currentGameObject->positions[instanceIndex] = glm::vec3(currentGameObject->positions[instanceIndex].x, currentGameObject->positions[instanceIndex].y, 0.0f);
    }
    else if (pMin.z <= 0.0f && currentGameObject->directions[instanceIndex] == glm::vec3(0.0f, 0.0f, -1.0f)) {
        currentGameObject->positions[instanceIndex] = glm::vec3(currentGameObject->positions[instanceIndex].x, currentGameObject->positions[instanceIndex].y, static_cast<float>(columnDim) - 1.0f);
    }
}

void VulnerableGhost::syncGhosts(const bool syncThis) const {
    auto& positions = drawBlue
        ? gameObjectsBlue[currentModelIndex]->positions
        : gameObjectsWhite[currentModelIndex]->positions;

    auto& directions = drawBlue
        ? gameObjectsBlue[currentModelIndex]->directions
        : gameObjectsWhite[currentModelIndex]->directions;

    if (syncThis) {
        if (ghostMapping.blinkyIndex != -1) {
            positions[ghostMapping.blinkyIndex] = blinky->gameObject->positions[0];
            directions[ghostMapping.blinkyIndex] = blinky->gameObject->directions[0];
        }
        if (ghostMapping.clydeIndex != -1) {
            positions[ghostMapping.clydeIndex] = clyde->gameObject->positions[0];
            directions[ghostMapping.clydeIndex] = clyde->gameObject->directions[0];
        }
        if (ghostMapping.inkyIndex != -1) {
            positions[ghostMapping.inkyIndex] = inky->gameObject->positions[0];
            directions[ghostMapping.inkyIndex] = inky->gameObject->directions[0];
        }
        if (ghostMapping.pinkyIndex != -1) {
            positions[ghostMapping.pinkyIndex] = pinky->gameObject->positions[0];
            directions[ghostMapping.pinkyIndex] = pinky->gameObject->directions[0];
        }

        for (int i = 0; i < 4; ++i) {
            if (i == ghostMapping.blinkyIndex ||
                i == ghostMapping.clydeIndex ||
                i == ghostMapping.inkyIndex ||
                i == ghostMapping.pinkyIndex) {
                UpdateOtherGameObjects(i);
            }
        }
    }
    else {
        if (ghostMapping.blinkyIndex != -1) {
            blinky->gameObject->positions[0] = positions[ghostMapping.blinkyIndex];
            blinky->gameObject->directions[0] = directions[ghostMapping.blinkyIndex];
        }
        if (ghostMapping.clydeIndex != -1) {
            clyde->gameObject->positions[0] = positions[ghostMapping.clydeIndex];
            clyde->gameObject->directions[0] = directions[ghostMapping.clydeIndex];
        }
        if (ghostMapping.inkyIndex != -1) {
            inky->gameObject->positions[0] = positions[ghostMapping.inkyIndex];
            inky->gameObject->directions[0] = directions[ghostMapping.inkyIndex];
        }
        if (ghostMapping.pinkyIndex != -1) {
            pinky->gameObject->positions[0] = positions[ghostMapping.pinkyIndex];
            pinky->gameObject->directions[0] = directions[ghostMapping.pinkyIndex];
        }
    }
}

void VulnerableGhost::syncAlternationGhosts() const {
    const int numInstances = static_cast<int>(this->GetCurrentGameObject()->GetNumInstances());
    for (int j = 0; j < numInstances; ++j) {
        if (drawBlue) {
            this->gameObjectsWhite[this->currentModelIndex]->positions[j] = this->gameObjectsBlue[this->currentModelIndex]->positions[j];
            this->gameObjectsWhite[this->currentModelIndex]->directions[j] = this->gameObjectsBlue[this->currentModelIndex]->directions[j];
            this->UpdateOtherGameObjects(j);
        }
        else {
            this->gameObjectsBlue[this->currentModelIndex]->positions[j] = this->gameObjectsWhite[this->currentModelIndex]->positions[j];
            this->gameObjectsBlue[this->currentModelIndex]->directions[j] = this->gameObjectsWhite[this->currentModelIndex]->directions[j];
            this->UpdateOtherGameObjects(j);
        }
    }
}
