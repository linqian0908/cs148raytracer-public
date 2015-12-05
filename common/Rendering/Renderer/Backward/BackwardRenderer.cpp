#include "common/Rendering/Renderer/Backward/BackwardRenderer.h"
#include "common/Scene/Scene.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Scene/Lights/Light.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Intersection/IntersectionState.h"

BackwardRenderer::BackwardRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) :
    Renderer(scene, sampler)
{
}

void BackwardRenderer::InitializeRenderer()
{
}

glm::vec3 BackwardRenderer::ComputeSampleColor(const IntersectionState& intersection, const Ray& fromCameraRay) const
{
    if (!intersection.hasIntersection) {
        return glm::vec3();
    }

    glm::vec3 intersectionPoint = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
    const MeshObject* parentObject = intersection.intersectedPrimitive->GetParentMeshObject();
    assert(parentObject);

    const Material* objectMaterial = parentObject->GetMaterial();
    assert(objectMaterial);

    // Compute the color at the intersection.
    glm::vec3 sampleColor;
    for (size_t i = 0; i < storedScene->GetTotalLights(); ++i) {
        const Light* light = storedScene->GetLightObject(i);
        assert(light);

        // Sample light using rays, Number of samples and where to sample is determined by the light.
        std::vector<Ray> sampleRays;
        light->ComputeSampleRays(sampleRays, intersectionPoint, intersection.ComputeNormal());

        for (size_t s = 0; s < sampleRays.size(); ++s) {
            // note that max T should be set to be right before the light.
            IntersectionState state(0,0);
            bool didIntersect;
            bool hit = false;
            glm::vec3 color = light->GetLightColor();
            float bounces=10;
            do {
                bounces--;               
                didIntersect=storedScene->Trace(&sampleRays[s], &state);
                if (!didIntersect) {
                    break;
                }
                
                else {
                    const MeshObject* hitMesh = state.intersectedPrimitive->GetParentMeshObject();
                    assert(hitMesh);
                    const Material* hitMaterial = hitMesh->GetMaterial();
                    assert(hitMaterial);
                    
                    if (!hitMaterial->IsTransmissive()) {
                        hit=true;
                        break;
                    }
                    else {
                        const glm::vec3 hitPoint = state.intersectionRay.GetRayPosition(state.intersectionT);
                        sampleRays[s].SetRayPosition(hitPoint+LARGE_EPSILON*sampleRays[s].GetRayDirection());
                        sampleRays[s].SetMaxT(sampleRays[s].GetMaxT()-state.intersectionT);
                        
                        glm::vec3 dt=hitMaterial->GetBaseTransmittance();
                        color.x *= std::sqrt(dt.x);
                        color.y *= std::sqrt(dt.y);
                        color.z *= std::sqrt(dt.z);
                    }
                }
            } while(bounces>0);
            
            if (hit) {
                continue;
            }
            
            const float lightAttenuation = light->ComputeLightAttenuation(intersectionPoint);
            // Note that the material should compute the parts of the lighting equation too.
            const glm::vec3 brdfResponse = objectMaterial->ComputeBRDF(intersection,color, sampleRays[s], fromCameraRay, lightAttenuation);
            sampleColor += brdfResponse;
        }
    }
    sampleColor += objectMaterial->ComputeNonLightDependentBRDF(this, intersection);
    return sampleColor;
}
