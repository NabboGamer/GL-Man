#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Ghost.hpp"
#include "GameObjectBase.hpp"

class Blinky : public Ghost {

public:
	GameObjectBase* gameObject;
	const float BLINKY_SPEED = 2.5f;
	const float MAX_DT = 0.016f;  //Equivalent to a frame rate of around 60 FPS

	Blinky();
	~Blinky();

	void Move(double dt, GameObjectBase* mazeWall) override;
	void Draw(double deltaTime) override;

private:
	void init() override;
	bool doCollisions(GameObjectBase* mazeWall);

};