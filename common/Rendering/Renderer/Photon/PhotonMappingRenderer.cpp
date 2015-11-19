#include "common/Rendering/Renderer/Photon/PhotonMappingRenderer.h"
#include "common/Scene/Scene.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Scene/Lights/Light.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Intersection/IntersectionState.h"
#include "common/Scene/SceneObject.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "glm/gtx/component_wise.hpp"

#define VISUALIZE_PHOTON_MAPPING 1

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000),
    maxPhotonBounces(1000)
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    GenericPhotonMapGeneration(diffuseMap, diffusePhotonNumber);
    diffuseMap.optimise();
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons)
{
    float totalLightIntensity = 0.f;
    size_t totalLights = storedScene->GetTotalLights();
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }
        totalLightIntensity = glm::length(currentLight->GetLightColor());
    }

    // Shoot photons -- number of photons for light is proportional to the light's intensity relative to the total light intensity of the scene.
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }

        const float proportion = glm::length(currentLight->GetLightColor()) / totalLightIntensity;
        const int totalPhotonsForLight = static_cast<const int>(proportion * totalPhotons);
        const glm::vec3 photonIntensity = currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);
        for (int j = 0; j < totalPhotonsForLight; ++j) {
            Ray photonRay;
            std::vector<char> path;
            path.push_back('L');
            currentLight->GenerateRandomPhotonRay(photonRay);
            TracePhoton(photonMap, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
        }
    }
}

void PhotonMappingRenderer::TracePhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
{
    /*
     * Assignment 7 TODO: Trace a photon into the scene and make it bounce.
     *    
     *    How to insert a 'Photon' struct into the photon map.
     *        Photon myPhoton;
     *        ... set photon properties ...
     *        photonMap.insert(myPhoton);
     */
    assert(photonRay);
    IntersectionState state(0, 0);
    state.currentIOR = currentIOR;
    /* my code starts here */
    if (remainingBounces>=0) {
        storedScene->Trace(photonRay,&state);
        if (state.hasIntersection) {// intersecting Scene object
            const glm::vec3 intersectionPoint=state.intersectionRay.GetRayPosition(state.intersectionT);
            
            if (path.size()>1) {// store photon
                Photon newPhoton;
                newPhoton.position=intersectionPoint;
                newPhoton.intensity=lightIntensity;
                const Ray toLightRay=Ray(intersectionPoint,-photonRay->GetRayDirection());
                newPhoton.toLightRay=toLightRay;
                photonMap.insert(newPhoton);
            }
    
            // photon scattering/absorption
            const MeshObject* hitMeshObject=state.intersectedPrimitive->GetParentMeshObject();
            const Material* hitMaterial=hitMeshObject->GetMaterial();
            glm::vec3 d=hitMaterial->GetBaseDiffuseReflection();
            float pr=std::max(d.x,d.y);
            pr=std::max(pr,d.z);
            float r=std::rand()*1.f/RAND_MAX;
            if (r<pr) {// scatter photon
                // hemisphere sampling
                float u1=std::rand()*1.f/RAND_MAX;
                float u2=std::rand()*1.f/RAND_MAX;
                float r=std::sqrt(u1);
                float theta=2*PI*u2;
                float x=r*std::cos(theta);
                float y=r*std::sin(theta);
                float z=std::sqrt(1-u1);
                // hemisphere ray transformation
                glm::vec3 n=state.ComputeNormal();
                glm::vec3 t;
                glm::vec3 b;
                if (std::abs(n.x)<0.9f) {
                    t=glm::normalize(glm::vec3(0.0,n.z,-n.y));
                    b=glm::normalize(glm::vec3(-n.z*n.z-n.y*n.y,n.x*n.y,n.x*n.z));
                }
                else {
                    t=glm::normalize(glm::vec3(-n.z,0.0,n.x));
                    b=glm::normalize(glm::vec3(n.x*n.y,-n.x*n.x-n.z*n.z,n.z*n.y));
                }
                const glm::vec3 rayDirection=glm::normalize(glm::vec3(x*t.x+y*t.y+z*t.z,x*b.x+y*b.y+z*b.z,x*n.x+y*n.y+z*n.z));
                photonRay->SetRayDirection(rayDirection); 
                const glm::vec3 rayPosition=intersectionPoint+n*LARGE_EPSILON;
                photonRay->SetRayPosition(rayPosition);            
                path.push_back('S');
                PhotonMappingRenderer::TracePhoton(photonMap, photonRay, lightIntensity, path, currentIOR, remainingBounces-1);
           }
       }
   }            
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);
#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    std::vector<Photon> foundPhotons;
    diffuseMap.find_within_range(intersectionVirtualPhoton, 0.003f, std::back_inserter(foundPhotons));
    if (!foundPhotons.empty()) {
        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
    }
#endif
    return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}
