#pragma once

#include <glm/glm.hpp>

class Utility {

public:
    // Function to round a float to six decimal places
    static float     ApproximateFloatToSixDecimals(float value);

    // Function to round a 3D vector to six decimal places
    static glm::vec3 ApproximateVec3ToSixDecimals(glm::vec3 vec);

private:
    // Private constructor to prevent a new instance from being created from outside
    Utility();
};
