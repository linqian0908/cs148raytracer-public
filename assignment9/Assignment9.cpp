#include "assignment9/Assignment9.h"
#include "common/core.h"

std::shared_ptr<Camera> Assignment9::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    return camera;
}

std::shared_ptr<Scene> Assignment9::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

// cube
    // Material
    std::shared_ptr<BlinnPhongMaterial> defaultMaterial = std::make_shared<BlinnPhongMaterial>();
    defaultMaterial->SetDiffuse(glm::vec3(0.5f,.5f,0.f));
    
    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> objects = MeshLoader::LoadMesh("table.obj", &loadedMaterials);
    std::cout<<objects.size()<<std::endl;
    for (size_t i = 0; i < objects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = defaultMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        switch(i) {
            case 4: // grape leaf
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/grape_leaf.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/grape_leaf.jpg"));
                break;                          
            case 7: // apple
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/apple.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/apple.jpg"));
                break;
            case 8: //bucket
                materialCopy->SetReflectivity(0.2);
                break;
            case 9:  //WINE BOTTLE
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/winebottle.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/winebottle.jpg"));
                materialCopy->SetTransmittance(0.2);
                break;
            case 10:  //WINE laebl
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/wine_label.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/wine_label.jpg"));
                break;
            case 11:  //WINE cork
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/cork.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/cork.jpg"));
                break;
            case 12:  //table
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/table_cloth.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/table_cloth.jpg"));
                break;
            case 13:  // glass
                materialCopy->SetTransmittance(0.9);
                materialCopy->SetIOR(1.46);
                break;
            case 1:  // candlebot
            case 2:
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/metal_brown.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/metal_brown.jpg"));
                break;
            case 0:  // candle
            case 14:
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/wax.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/wax.jpg"));
                break;
            case 15: // background
                materialCopy->SetTexture("diffuseTexture", TextureLoader::LoadTexture("textures/bg_gold.jpg"));
                materialCopy->SetTexture("specularTexture", TextureLoader::LoadTexture("textures/bg_gold.jpg"));
                break;
            default:
                break;
        }
        objects[i]->SetMaterial(materialCopy);
        
        std::shared_ptr<SceneObject> newObject = std::make_shared<SceneObject>();
        newObject->AddMeshObject(objects[i]);
        newObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
        newObject->MultScale(.2f);
        newObject->Translate(glm::vec3(0.1f,0.f,0.f));
        newObject->CreateAccelerationData(AccelerationTypes::BVH);
        
        newScene->AddSceneObject(newObject);
    }
    
    // Lights
    //std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();  
    //std::shared_ptr<AreaLight> pointLight = std::make_shared<AreaLight>(glm::vec2(0.5f, 0.5f));
    //pointLight->SetSamplerAttributes(glm::vec3(3.f, 3.f, 1.f), 9);
    std::shared_ptr<Light> pointLight = std::make_shared<DirectionalLight>();
    pointLight->Rotate(glm::vec3(1.f,-1.f,0),PI/6);
    pointLight->SetLightColor(glm::vec3(.6f, .6f, .6f));
    newScene->AddLight(pointLight);
    
    pointLight = std::make_shared<DirectionalLight>();
    pointLight->Rotate(glm::vec3(1.3f,-1.f,0),PI/6);
    pointLight->SetLightColor(glm::vec3(.3f, .3f, .3f));
    newScene->AddLight(pointLight);
    
    pointLight = std::make_shared<DirectionalLight>();
    pointLight->Rotate(glm::vec3(.7f,-1.f,0),PI/6);
    pointLight->SetLightColor(glm::vec3(.3f, .3f, .3f));
    newScene->AddLight(pointLight);
    
    return newScene;

}
std::shared_ptr<ColorSampler> Assignment9::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    jitter->SetGridSize(glm::ivec3(2, 2, 1));
    return jitter;
}

std::shared_ptr<class Renderer> Assignment9::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
    //return std::make_shared<BackwardRenderer>(scene, sampler);
    return std::make_shared<PhotonMappingRenderer>(scene, sampler);
}

int Assignment9::GetSamplesPerPixel() const
{
    // ASSIGNMENT 5 TODO: Change the '1' here to increase the maximum number of samples used per pixel. (Part 1).
    return 1; 
}

bool Assignment9::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int Assignment9::GetMaxReflectionBounces() const
{
    return 2;
}

int Assignment9::GetMaxRefractionBounces() const
{
    return 4;
}

glm::vec2 Assignment9::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}
