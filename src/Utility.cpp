#include "Utility.hpp"
#include <cmath>

Utility::Utility() {
    // Empty constructor, to avoid external creation
}

float Utility::approximateFloatToSixDecimals(float value) {
    float roundedValue = std::round(value * 1e6) / 1e6;
    // If the value is close to zero (positive or negative), let's approximate it to zero
    return (std::abs(roundedValue) < 1e-6f) ? 0.0f : roundedValue;
}

// Apply rounding to six decimal places on each component of glm::vec3
glm::vec3 Utility::approximateVec3ToSixDecimals(glm::vec3 vec) {
    return glm::vec3(approximateFloatToSixDecimals(vec.x),
                     approximateFloatToSixDecimals(vec.y),
                     approximateFloatToSixDecimals(vec.z));
}
