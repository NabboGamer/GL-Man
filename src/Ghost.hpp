#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "custom_types.hpp"
#include "GameObjectBase.hpp"

class Ghost {

public:

	Ghost();
	~Ghost();

	virtual void Move(double deltaTime, GameObjectBase* mazeWall) = 0;
	virtual void Draw(double deltaTime) = 0;

protected:
	virtual void init() = 0;
	bool         checkCollision(const CustomTypes::obb& obb1, const CustomTypes::obb& obb2);
	glm::vec3    resolveCollision(const CustomTypes::obb& playerObb, const CustomTypes::obb& wallObb);
};