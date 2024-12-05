#include "Utility.hpp"
#include <cmath>
#include <random>

std::vector<glm::vec3> Utility::possibleDirections = {
    glm::vec3( 1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3( 0.0f, 0.0f, 1.0f),
    glm::vec3( 0.0f, 0.0f,-1.0f)
};

Utility::Utility() = default;

float Utility::ApproximateFloatToSixDecimals(const float value) {
    const float roundedValue = static_cast<float>(std::round(value * 1e6) / 1e6);
    // If the value is close to zero (positive or negative), let's approximate it to zero
    return (std::abs(roundedValue) < 1e-6f) ? 0.0f : roundedValue;
}

// Apply rounding to six decimal places on each component of glm::vec3
glm::vec3 Utility::ApproximateVec3ToSixDecimals(const glm::vec3 vec) {
    return {ApproximateFloatToSixDecimals(vec.x),
    	      ApproximateFloatToSixDecimals(vec.y),
    	      ApproximateFloatToSixDecimals(vec.z)};
}

glm::vec3 Utility::GetRandomDirection() {
    // Random generator
    std::random_device rd;                                   // Source of entropy
    std::mt19937 gen(rd());                              // Mersenne Twister for random generation
    std::uniform_int_distribution<> dist(0, 3);     // Uniform distribution for an index between 0 and 3

    const int randomIndex = dist(gen);                    // Get a random index
    return Utility::possibleDirections[randomIndex];         // Returns the chosen direction
}

glm::vec3 Utility::NormalizeZeros(const glm::vec3& vec) {
    glm::vec3 result = vec;
    if (result.x == -0.0f) result.x = 0.0f;
    if (result.y == -0.0f) result.y = 0.0f;
    if (result.z == -0.0f) result.z = 0.0f;
    return result;
}

std::size_t Utility::Vec3Hash::operator()(const glm::vec3& v) const {
    std::size_t h1 = std::hash<float>()(v.x);
    std::size_t h2 = std::hash<float>()(v.y);
    std::size_t h3 = std::hash<float>()(v.z);
    h1 ^= h2 << 1;
    h1 ^= h3 << 2;
    return h1;
}