#pragma once

#include "common/Rendering/Renderer.h"
#include "common/Rendering/Renderer/Photon/Photon.h"
#include <kdtree++/kdtree.hpp>
#include <functional>
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Renderer/Backward/BackwardRenderer.h"

class PhotonMappingRenderer : public BackwardRenderer
{
public:
    PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler);
    virtual void InitializeRenderer() override;
    glm::vec3 ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const override;

    void SetNumberOfDiffusePhotons(int diffuse);
    void SetNumberOfCausticPhotons(int caustic);
private:
    using PhotonKdtree = KDTree::KDTree<3, Photon, PhotonAccessor>;
    PhotonKdtree diffuseMap;
    PhotonKdtree causticMap;

    int diffusePhotonNumber;
    int causticPhotonNumber;
    int diffuseTotal;
    int causticTotal;
    int maxPhotonBounces;

    void GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons, int type);
    bool TraceGlobalPhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces);    
    bool TraceCausticPhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces);
    void StorePhoton(PhotonKdtree& photonMap, glm::vec3 intersectionPoint, glm::vec3 intensity, Ray* photonRay, glm::vec3 normal);
    void PerformRaySpecularReflection(Ray& outputRay, const Ray& inputRay, const glm::vec3& intersectionPoint, const float NdR, const IntersectionState& state) const;
    void PerformRayRefraction(Ray& outputRay, const Ray& inputRay, const glm::vec3& intersectionPoint, const float NdR, const IntersectionState& state, float& targetIOR) const;
};
