#pragma once

#include <vector>

#include "GameObjectBase.hpp"

class PacMan {

public:
	std::vector<GameObjectBase*> gameObjects;

	PacMan();
	~PacMan();

	int  GetCurrentModelIndex() const;
	void UpdateOtherGameObjects() const;
	void Draw(double deltaTime);

private:
	const double frameDuration   = 0.04; // 0.04 seconds corresponds to 1/25 of a second, therefore the model will be changed 25 times per second
	const float MAX_DT = 0.016f;         // Equivalent to a frame rate of around 60 FPS
	double timeAccumulator = 0.0f;
	int currentModelIndex = 0;
	

	void init();
};
