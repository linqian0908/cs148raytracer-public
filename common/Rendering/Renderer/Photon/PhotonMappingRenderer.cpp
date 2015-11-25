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

#define VISUALIZE_PHOTON_MAPPING 0
#define CAUSTIC 0

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000),
    causticPhotonNumber(100000),
    maxPhotonBounces(5)
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    GenericPhotonMapGeneration(diffuseMap, diffusePhotonNumber,0);
    diffuseMap.optimise();
    std::cout<<"finish initialize Global photon map"<<std::endl;
#if CAUSTIC
    GenericPhotonMapGeneration(causticMap, causticPhotonNumber,1);
    causticMap.optimise():
    std::cout<<"finish initialize Caustic photon map"<<std::endl;
#endif

    
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons, int type)
{
    float totalLightIntensity = 0.f;
    size_t totalLights = storedScene->GetTotalLights();
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }
        totalLightIntensity += glm::length(currentLight->GetLightColor());
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
            switch (type) {
                case 0:
                    TraceGlobalPhoton(photonMap, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
                    break;
                case 1:
                    if(!TraceCausticPhoton(photonMap,&photonRay, photonIntensity, path, 1.f, maxPhotonBounces)) {
                        j--;
                    }
                    break;
            }
        }
    }
}

void PhotonMappingRenderer::TraceGlobalPhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
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
                StorePhoton(photonMap,intersectionPoint,lightIntensity,photonRay,state.ComputeNormal());
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
                path.push_back('D');
                // rescale color of scattered photon
                lightIntensity.x *= d.x/pr;
                lightIntensity.y *= d.y/pr;
                lightIntensity.z *= d.z/pr;
                PhotonMappingRenderer::TraceGlobalPhoton(photonMap, photonRay, lightIntensity, path, currentIOR, remainingBounces-1);
           }
       }
   }            
}

bool PhotonMappingRenderer::TraceCausticPhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
{
    assert(photonRay);
    IntersectionState state(0, 0);
    state.currentIOR = currentIOR;
    
    storedScene->Trace(photonRay,&state);
    if (!state.hasIntersection) {
        if (path.size()==1) {return false;}
        else {return true;}
    }
    else {// intersecting Scene object
        const MeshObject* hitMeshObject=state.intersectedPrimitive->GetParentMeshObject();
        const Material* hitMaterial=hitMeshObject->GetMaterial(); 
        
        // first object hit is not a specular
        if (path.size()==1 && !hitMaterial->IsReflective() && !hitMaterial->IsTransmissive()) {
            return false;
        }
        
        const glm::vec3 intersectionPoint=state.intersectionRay.GetRayPosition(state.intersectionT);
        const glm::vec3 normal=state.ComputeNormal();
        // hit diffusive object after specular object: add photon to map
        if (path.size()>1 && hitMaterial->HasDiffuseReflection()) {// store photon
            StorePhoton(photonMap,intersectionPoint,lightIntensity,photonRay,normal);
        }
        
        if (remainingBounces>=1) {
            // decide whether to reflect and refract photon
            path.push_back('S');
            const float NdR = glm::dot(photonRay->GetRayDirection(), normal);
            if (hitMaterial->IsReflective()) {
                Ray reflectionRay;
                PerformRaySpecularReflection(reflectionRay, *photonRay,intersectionPoint,NdR,state);
                glm::vec3 ds=hitMaterial->GetBaseSpecularReflection();
                const glm::vec3 reflectIntensity(ds.x*lightIntensity.x,ds.y*lightIntensity.y,ds.z*lightIntensity.z);
                TraceCausticPhoton(photonMap, &reflectionRay, reflectIntensity, path, currentIOR, remainingBounces-1);
            }
            if (hitMaterial->IsTransmissive()) {
                float targetIOR = (NdR < SMALL_EPSILON) ? hitMaterial->GetIOR():1.f;                
                Ray refractionRay;
                PerformRayRefraction(refractionRay, *photonRay,intersectionPoint,NdR,state,targetIOR);
                glm::vec3 dt=hitMaterial->GetBaseTransmittance();
                const glm::vec3 refractionIntensity(dt.x*lightIntensity.x,dt.y*lightIntensity.y,dt.z*lightIntensity.z);                
                TraceCausticPhoton(photonMap, &refractionRay, refractionIntensity, path, targetIOR, remainingBounces-1);
            }
        }
        return true;
    }
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);

    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
    glm::vec3 intersectNormal=intersection.ComputeNormal();

    std::vector<Photon> foundPhotons;
    float sampleRadius=0.03f;
    diffuseMap.find_within_range(intersectionVirtualPhoton, sampleRadius, std::back_inserter(foundPhotons));    
#if CAUSTIC
    causticMap.find_within_range(intersectionVirtualPhoton, sampleRadius, std::back_inserter(foundPhotons));
#endif

    if (!foundPhotons.empty()) {
#if VISUALIZE_PHOTON_MAPPING
        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
#else
        Photon samplePhoton;
        glm::vec3 sampleColor;
        
        const MeshObject* parentObject = intersection.intersectedPrimitive->GetParentMeshObject();
        assert(parentObject);
        const Material* objectMaterial = parentObject->GetMaterial();
        assert(objectMaterial);            
        
        IntersectionState sampleIntersection(0,0);
        size_t used=0;
        for (size_t s=0; s<foundPhotons.size(); s++) {
            samplePhoton=foundPhotons[s];
            if (glm::dot(intersectNormal,samplePhoton.normal)>0.5) {//filtering by normal
                const glm::vec3 brdfResponse = objectMaterial->ComputeBRDF(intersection, samplePhoton.intensity, samplePhoton.toLightRay, fromCameraRay, 1.f);
                sampleColor += brdfResponse;
                used++;
            }
            //else { std::cout << glm::dot(intersectNormal,samplePhoton.normal) << std::endl;}
        }
        sampleColor /= (used*PI*sampleRadius*sampleRadius/foundPhotons.size()); 
        //if (used<foundPhotons.size()) { std::cout << used << ", "<<foundPhotons.size() << std::endl;}
        //std::cout << sampleColor.x << ", " << sampleColor.y << ", " << sampleColor.z << std::endl;
        //std::cout << finalRenderColor.x << ", " << finalRenderColor.y << ", " << finalRenderColor.z << std::endl;
        finalRenderColor += sampleColor;
#endif
    }
    return finalRenderColor;
}

void PhotonMappingRenderer::StorePhoton(PhotonKdtree& photonMap, glm::vec3 intersectionPoint, glm::vec3 intensity, Ray* photonRay, glm::vec3 normal) {
    Photon newPhoton;
    newPhoton.position=intersectionPoint;
    newPhoton.intensity=intensity;
    const Ray toLightRay=Ray(intersectionPoint,-photonRay->GetRayDirection());
    newPhoton.toLightRay=toLightRay;
    newPhoton.normal=normal;
    photonMap.insert(newPhoton);
}

void PhotonMappingRenderer::PerformRaySpecularReflection(Ray& outputRay, const Ray& inputRay, const glm::vec3& intersectionPoint, const float NdR, const IntersectionState& state) const
{
    const glm::vec3 normal = (NdR > SMALL_EPSILON) ? -1.f * state.ComputeNormal() : state.ComputeNormal();
    const glm::vec3 reflectionDir = glm::reflect(inputRay.GetRayDirection(), normal);
    outputRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * state.ComputeNormal());
    outputRay.SetRayDirection(reflectionDir);
}

void PhotonMappingRenderer::PerformRayRefraction(Ray& outputRay, const Ray& inputRay, const glm::vec3& intersectionPoint, const float NdR, const IntersectionState& state, float& targetIOR) const
{
    const glm::vec3 refractionDir = inputRay.RefractRay(state.ComputeNormal(), state.currentIOR, targetIOR);
    outputRay.SetRayPosition(intersectionPoint + LARGE_EPSILON * state.ComputeNormal());
    outputRay.SetRayDirection(refractionDir);
}


void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}

void PhotonMappingRenderer::SetNumberOfCausticPhotons(int caustic)
{
    causticPhotonNumber = caustic;
}
