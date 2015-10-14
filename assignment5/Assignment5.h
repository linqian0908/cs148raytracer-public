#pragma once

#include "common/Application.h"

class Assignment5 : public Application
{
public:
    virtual std::shared_ptr<class Camera> CreateCamera() const override;
    virtual std::shared_ptr<class Scene> CreateScene() const override;
    virtual std::shared_ptr<class Sampler> CreateSampler() const override;
    virtual int GetSamplesPerPixel() const override;
    virtual bool NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex);
private:
};