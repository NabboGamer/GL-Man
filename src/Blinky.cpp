#include <windows.h>

#include "Blinky.hpp"
#include "custom_types.hpp"
#include "Utility.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"
#include "GameObjectFromModel.hpp"

Blinky::Blinky() : Ghost() {
	this->init();
}

Blinky::~Blinky() {
	delete gameObject;
}

void Blinky::Move(double dt, GameObjectBase* mazeWall) {
	float effectiveDt = std::min(static_cast<float>(dt), MAX_DT);
	float speed = BLINKY_SPEED * effectiveDt;
	this->gameObject->positions[0] += speed * this->gameObject->directions[0];
	bool doesItCollide = this->doCollisions(mazeWall);
	if (doesItCollide) {
		do {
			this->gameObject->directions[0] = Utility::GetRandomDirection();
			this->gameObject->positions[0] += speed * this->gameObject->directions[0];
			doesItCollide = this->doCollisions(mazeWall);
		} while (doesItCollide == true);
	}
}

void Blinky::Draw(double deltaTime) {
	this->gameObject->Draw();
}

void Blinky::init() {
	std::vector<glm::vec3> blinkyPositions  = { glm::vec3(19.0f, 0.0f, 12.0f) };
	std::vector<glm::vec3> blinkyDirections = { glm::vec3(0.0f, 0.0f, 1.0f) };
	std::vector<float>     blinkyRotations  = { 0.0f };
	std::vector<glm::vec3> blinkyScaling    = { glm::vec3(0.25f) };

	ResourceManager::LoadModel("../res/objects/ghosts/blinky/blinky.obj", "blinkyModel");
	this->gameObject = new GameObjectFromModel(blinkyPositions,
											   blinkyDirections,
											   blinkyRotations,
											   blinkyScaling,
											   &ResourceManager::GetShader("ghostShader"),
											   &ResourceManager::GetModel("blinkyModel"));
}

bool Blinky::doCollisions(GameObjectBase* mazeWall) {
	auto blinkyObb = this->gameObject->GetTransformedBoundingBox(0);

	// CHECK COLLISION BLINKY-WALL
	size_t numInstancesMazeWall = mazeWall->GetNumInstances();
	for (size_t i = 0; i < numInstancesMazeWall; i++) {
		auto mazeWallObb = mazeWall->GetTransformedBoundingBox(i);
		bool collision = this->checkCollision(blinkyObb, mazeWallObb);
		if (collision) {
			LoggerManager::LogDebug("There was a collision between BLINKY and WALL number {}", i);
			return true;
		}
	}
	return false;
}
