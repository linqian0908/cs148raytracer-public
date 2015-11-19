#pragma once

#include "common/Scene/SceneObject.h"

class Camera : public SceneObject
{
public:
    Camera();

    virtual std::shared_ptr<class Ray> GenerateRayForNormalizedCoordinates(glm::vec2 coordinate) const = 0;
    virtual std::shared_ptr<class Ray> GenerateRandomRayFromLenArea(glm::vec2 coordinate, glm::vec3 standardRayDirection) const = 0;
};
