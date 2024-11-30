#pragma once

#include <utility>
#include <glm/vec3.hpp>

namespace CustomTypes {

    // Defines an Oriented Bounding Box (OBB)
    typedef std::pair<glm::vec3, glm::vec3> obb;

}

namespace CustomStructs {

    struct Config {
        bool useMSAA = false;
        unsigned int numSampleMSAA = 4;
        bool useHDR = false;
        float exposure = 0.5f;
    };
	
}
