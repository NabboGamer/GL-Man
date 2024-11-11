#include "Utility.hpp"
#include <cmath>
#include <random>

Utility::Utility() {
    // Empty constructor, to avoid external creation
}

float Utility::ApproximateFloatToSixDecimals(float value) {
    float roundedValue = static_cast<float>(std::round(value * 1e6) / 1e6);
    // If the value is close to zero (positive or negative), let's approximate it to zero
    return (std::abs(roundedValue) < 1e-6f) ? 0.0f : roundedValue;
}

// Apply rounding to six decimal places on each component of glm::vec3
glm::vec3 Utility::ApproximateVec3ToSixDecimals(glm::vec3 vec) {
    return glm::vec3(ApproximateFloatToSixDecimals(vec.x),
                     ApproximateFloatToSixDecimals(vec.y),
                     ApproximateFloatToSixDecimals(vec.z));
}

glm::vec3 Utility::GetRandomDirection() {
    // Array of possible directions
    glm::vec3 directions[] = {
        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  0.0f, -1.0f)
    };

    // Random generator
    std::random_device rd;                           // Source of entropy
    std::mt19937 gen(rd());                          // Mersenne Twister for random generation
    std::uniform_int_distribution<> dist(0, 3);      // Uniform distribution for an index between 0 and 3

    int randomIndex = dist(gen);                     // Get a random index
    return directions[randomIndex];                  // Returns the chosen direction
}
