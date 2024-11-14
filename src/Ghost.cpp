#include "Ghost.hpp"

Ghost::Ghost() = default;

Ghost::~Ghost() = default;

// OBB - OBB collision detection in XZ plane
bool Ghost::checkCollision(const CustomTypes::obb& obb1, const CustomTypes::obb& obb2) {
	const glm::vec3 box1_min = obb1.first;
	const glm::vec3 box1_max = obb1.second;
	const glm::vec3 box2_min = obb2.first;
	const glm::vec3 box2_max = obb2.second;

	// Check only on X and Z axes
	const bool overlapX = box1_max.x >= box2_min.x && box1_min.x <= box2_max.x;
	const bool overlapZ = box1_max.z >= box2_min.z && box1_min.z <= box2_max.z;

	return overlapX && overlapZ;
}

glm::vec3 Ghost::resolveCollision(const CustomTypes::obb& playerObb, const CustomTypes::obb& wallObb) {
    const glm::vec3 playerMin = playerObb.first;
    const glm::vec3 playerMax = playerObb.second;
    const glm::vec3 wallMin = wallObb.first;
    const glm::vec3 wallMax = wallObb.second;

    // Calculate the penetration depth on the X and Z axes
    const float overlapX = std::min(playerMax.x - wallMin.x, wallMax.x - playerMin.x);
    const float overlapZ = std::min(playerMax.z - wallMin.z, wallMax.z - playerMin.z);

    // Find the axis with the least penetration and use that value to resolve the collision
    if (overlapX < overlapZ) {
        if (playerMax.x < wallMax.x) {
            return {-overlapX, 0.0f, 0.0f};
        }
        else {
            return {overlapX, 0.0f, 0.0f};
        }
    }
    else {
        if (playerMax.z < wallMax.z) {
            return {0.0f, 0.0f, -overlapZ};
        }
        else {
            return {0.0f, 0.0f, overlapZ};
        }
    }
}
