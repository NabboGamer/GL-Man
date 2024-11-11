#include "Ghost.hpp"

Ghost::Ghost(){
}

Ghost::~Ghost() {
}

// OBB - OBB collision detection in XZ plane
bool Ghost::checkCollision(const CustomTypes::obb& obb1, const CustomTypes::obb& obb2) {
	glm::vec3 box1_min = obb1.first;
	glm::vec3 box1_max = obb1.second;
	glm::vec3 box2_min = obb2.first;
	glm::vec3 box2_max = obb2.second;

	// Check only on X and Z axes
	bool overlapX = box1_max.x >= box2_min.x && box1_min.x <= box2_max.x;
	bool overlapZ = box1_max.z >= box2_min.z && box1_min.z <= box2_max.z;

	return overlapX && overlapZ;
}
