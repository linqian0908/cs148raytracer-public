#include "common/Scene/Camera/Perspective/PerspectiveCamera.h"
#include "common/Scene/Geometry/Ray/Ray.h"

PerspectiveCamera::PerspectiveCamera(float aspectRatio, float inputFov):
    aspectRatio(aspectRatio), fov(inputFov * PI / 180.f), zNear(0.f), zFar(std::numeric_limits<float>::max())
{
}

std::shared_ptr<Ray> PerspectiveCamera::GenerateRayForNormalizedCoordinates(glm::vec2 coordinate) const
{
    // Send ray from the camera to the image plane -- make the assumption that the image plane is at z = 1 in camera space.
    const glm::vec3 rayOrigin = glm::vec3(GetPosition());

    // Figure out where the ray is supposed to point to. 
    // Imagine that a frustum exists in front of the camera (which we assume exists at a singular point).
    // Then, given the aspect ratio and vertical field of view we can determine where in the world the 
    // image plane will exist and how large it is assuming we know for sure that z = 1 (this is fairly arbitrary for now).
    const float planeHeight = std::tan(fov / 2.f) * 2.f;
    const float planeWidth = planeHeight * aspectRatio;

    // Assume that (0, 0) is the top left of the image which means that when coordinate is (0.5, 0.5) the 
    // pixel is directly in front of the camera...
    const float xOffset = planeWidth * (coordinate.x - 0.5f);
    const float yOffset = -1.f * planeHeight  * (coordinate.y - 0.5f);
    const glm::vec3 targetPosition = rayOrigin + glm::vec3(GetForwardDirection()) + glm::vec3(GetRightDirection()) * xOffset + glm::vec3(GetUpDirection()) * yOffset;

    const glm::vec3 rayDirection = glm::normalize(targetPosition - rayOrigin);
    return std::make_shared<Ray>(rayOrigin + rayDirection * zNear, rayDirection, zFar - zNear);
}

std::shared_ptr<Ray> PerspectiveCamera::GenerateRandomRayFromLenArea(glm::vec2 coordinate, glm::vec3 standardRayDirection) const
{
    // Assume focal plane is at focalPlaneZ
    float focalPlaneZ = 3.5;
    // Assume the radius of len is 0.5
    float lenRadius = 0.05;
    // image plane will exist and how large it is assuming we know for sure that z = 1 (this is fairly arbitrary for now).
    const float planeHeight = std::tan(fov / 2.f) * 2.f;
    const float planeWidth = planeHeight * aspectRatio;
    // Assume that (0, 0) is the top left of the image which means that when coordinate is (0.5, 0.5) the
    // pixel is directly in front of the camera...
    const float xOffset = planeWidth * (coordinate.x - 0.5f) ;
    const float yOffset = -1.f * planeHeight  * (coordinate.y - 0.5f);
    
    /* How to compute the focal point with standard ray (in world space) and focal plane (in camera space)? */
    // ray origin is at a random point on the len's area
    glm::vec3 center = glm::vec3(GetPosition());
    // focal point is the intersection of standard ray direction and focal plane
    glm::vec3 focalPoint = center + glm::vec3(GetForwardDirection()) * focalPlaneZ + glm::vec3(GetRightDirection()) * xOffset * focalPlaneZ + glm::vec3(GetUpDirection()) * yOffset * focalPlaneZ;
     
    // generate a random point in the circular area of the len
    float u1=1.0f * std::rand()/RAND_MAX *lenRadius;
    float u2=1.0f * std::rand()/RAND_MAX *lenRadius;
    float r=std::sqrt(u1);
    float theta=2*PI*u2;
    float x=r*std::cos(theta);
    float y=r*std::sin(theta);
    // world space to camera space
    glm::vec3 rayOrigin = center + glm::vec3(GetRightDirection()) * x + glm::vec3(GetUpDirection()) * y;
    // the random Ray Direction
    glm::vec3 rayDirection = focalPoint - rayOrigin;
    return std::make_shared<Ray>(rayOrigin + rayDirection * zNear, rayDirection, zFar - zNear);
}

void PerspectiveCamera::SetZNear(float input)
{
    zNear = input;
}

void PerspectiveCamera::SetZFar(float input)
{
    zFar = input;
}
