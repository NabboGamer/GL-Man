#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "GameObjectBase.hpp"

class PacMan {

public:
	std::vector<GameObjectBase*> gameObjects;

	PacMan(glm::vec3 cameraPos, glm::vec3 cameraAt, glm::mat4 view, glm::mat4 projection);
	~PacMan();

	int  GetCurrentModelIndex() const;
	void Draw(double deltaTime);

private:
	double timeAccumulator = 0.0f;
	// 0.04 seconds corresponds to 1/25 of a second, 
	// therefore the model will be changed 25 times per second
	double frameDuration   = 0.04;
	int currentModelIndex = 0;

	void init(glm::vec3 cameraPos, glm::vec3 cameraAt, glm::mat4 view, glm::mat4 projection);
	void updateOtherGameObjects();
};
