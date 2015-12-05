#include "common/Scene/Lights/Directional/DirectionalLight.h"

void DirectionalLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    const glm::vec3 rayDirection = -1.f * glm::vec3(GetForwardDirection());
    output.emplace_back(origin + normal * LARGE_EPSILON, rayDirection);
}

float DirectionalLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 1.f;
}

void DirectionalLight::GenerateRandomPhotonRay(Ray& ray) const
{
    float d=2.0f;
    glm::vec3 sample;
    sample.x=(std::rand()*1.f/RAND_MAX-.5f)*d;
    sample.y=(std::rand()*1.f/RAND_MAX-.5f)*d;
    sample.z=4.f;
    glm::vec3 lightPosition = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(sample,1.f));
    ray.SetRayPosition(lightPosition); 
    
    ray.SetRayDirection(glm::vec3(GetForwardDirection()));
}
