#pragma once

#include <glm/glm.hpp>
#include <functional>

class Utility {

public:

    static std::vector<glm::vec3> possibleDirections;

    // Function to round a float to six decimal places
    static float     ApproximateFloatToSixDecimals(float value);

    // Function to round a 3D vector to six decimal places
    static glm::vec3 ApproximateVec3ToSixDecimals(glm::vec3 vec);

    static glm::vec3 GetRandomDirection();

    static glm::vec3 NormalizeZeros(const glm::vec3& vec);

    // Custom hash function for glm::vec3
    struct Vec3Hash {
        std::size_t operator ()(const glm::vec3& v) const;
    };

private:
    // Private constructor to prevent a new instance from being created from outside
    Utility();
};
