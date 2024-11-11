#include <windows.h>

#include "PacMan.hpp"
#include "ResourceManager.hpp"
#include "GameObjectFromModel.hpp"
#include "LoggerManager.hpp"

PacMan::PacMan() {
	this->init();
}

PacMan::~PacMan() {
	for (auto gameObjectBasePtr : this->gameObjects) {
		delete gameObjectBasePtr;
	}
}

int PacMan::GetCurrentModelIndex() const {
	return this->currentModelIndex;
}

void PacMan::Draw(double deltaTime) {
	GameObjectBase* currentGameObject;
	this->timeAccumulator += deltaTime;
	// When timeAccumulator is greater than or equal to frameDuration, 
	// it means that enough time (0.04 seconds) has passed for the model to change.
	if (this->timeAccumulator >= this->frameDuration) {
		// If the accumulated time is, for example, 0.06 seconds, we exceed frameDuration 
		// (0.04 seconds) and would have to change the model, but there would be 0.02 seconds left. 
		// This remaining time will be accumulated in the next loop, so we subtract it to "reset" 
		// timeAccumulator without losing the remaining time
		timeAccumulator -= frameDuration;
		// This is a formula to cyclically increase the model index, moving to the next one.
		// % 10 ensures that once the last model is reached, the index resets to 0. 
		// This causes the animation to loop.
		this->currentModelIndex = (this->currentModelIndex + 1) % 10;
	}
	currentGameObject = this->gameObjects[this->currentModelIndex];
	currentGameObject->Draw();
	this->updateOtherGameObjects();
}

void PacMan::init() {
	std::vector<glm::vec3> pacmanPositions  = { glm::vec3(7.5f, 0.0f, 13.5f) };
	std::vector<glm::vec3> pacmanDirections = { glm::vec3(0.0f, 0.0f, -1.0f) };
	std::vector<float>     pacmanRotations  = { 0.0f };
	std::vector<glm::vec3> pacmanScaling    = { glm::vec3(0.20f) };
	
	for (int i = 0; i < 10; i++){
		std::string string = "pacman";
		std::string completeString = string + std::to_string(i + 1);
		ResourceManager::LoadModel("../res/objects/pacman/" + completeString + "/" + completeString + ".obj", completeString);
		this->gameObjects.push_back(new GameObjectFromModel(pacmanPositions,
															pacmanDirections,
															pacmanRotations,
															pacmanScaling,
															&ResourceManager::GetShader("pacmanShader"),
															&ResourceManager::GetModel(completeString)));
	}

}

void PacMan::updateOtherGameObjects() {
	for (int i = 0; i < 10; i++) {
		if (i != this->currentModelIndex) {
			this->gameObjects[i]->positions[0]  = this->gameObjects[this->currentModelIndex]->positions[0];
			this->gameObjects[i]->directions[0] = this->gameObjects[this->currentModelIndex]->directions[0];
			/*this->gameObjects[i]->rotations[0]  = this->gameObjects[currentModelIndex]->rotations[0];
			this->gameObjects[i]->scaling[0]    = this->gameObjects[currentModelIndex]->scaling[0];*/
		}
	}
}

