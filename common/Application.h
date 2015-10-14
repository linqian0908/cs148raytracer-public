#pragma once

#include "common/common.h"

enum class AccelerationTypes;

class Application : public std::enable_shared_from_this<Application>
{
public:
    virtual ~Application() {}
    virtual std::shared_ptr<class Camera> CreateCamera() const = 0;
    virtual std::shared_ptr<class Scene> CreateScene() const = 0;
    virtual std::shared_ptr<class Sampler> CreateSampler() const = 0;

    // Sampling Properties
    virtual int GetSamplesPerPixel() const;

    // whether or not to continue sampling the scene from the camera.
    virtual bool NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex) = 0;

    // Acceleration Properties
    virtual AccelerationTypes GetSceneAccelerationType() const;
    virtual AccelerationTypes GetPerObjectAccelerationType() const;

    // Postprocessing
    virtual void PerformImagePostprocessing(class ImageWriter& imageWriter);

    virtual std::string GetOutputFilename() const;
private:
};