#include "Utility.hpp"
#include <cmath>

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
