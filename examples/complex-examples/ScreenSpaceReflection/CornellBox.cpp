#include "CornellBox.h"
#include "FrameworkCore/SpecialModels/SpecialModel.h"
#include "../Base/BasicSettings.h"

using namespace OpenGLFramework::Core;

CornellBox::CornellBox(OpenGLFramework::Core::Shader& init):
    directShader_{ init }
{
    auto InsertNewQuad = [&models = models_](float degree, const glm::vec3& axis,
        const glm::vec3& translation)
    {
        auto wall = Quad::GetBasicTriRenderModel();
        wall.transform.Rotate(degree, axis)
                      .Translate(translation)
                      .scale = glm::vec3{ 5 };
        models.push_back(std::move(wall));
    };

    InsertNewQuad(90, { 0, 1, 0 }, { -5, 0, 0 }); // left wall
    InsertNewQuad(90, { 0, -1, 0 }, { 5, 0, 0 }); // right wall
    InsertNewQuad(90, { 1, 0, 0 }, { 0, 5, 0 }); // ceil
    InsertNewQuad(90, { -1, 0, 0 }, { 0, -5, 0 }); // floor
    InsertNewQuad(0, { 0, 1, 0 }, { 0, 0, -5 }); // back

    auto InsertCube = [&models = models_](float degree, const glm::vec3& axis,
        const glm::vec3& translation, const glm::vec3& scale)
    {
        auto box = Cube::GetBasicTriRenderModel();
        box.transform.Rotate(degree, axis)
                     .Translate(translation)
                     .scale = scale;
        models.push_back(std::move(box));
    };
    InsertCube(-18, { 0,1,0 }, { 1.2,-5,1.1}, glm::vec3{ 3 });
    InsertCube(15, { 0,1,0 }, { -3.2,-5,-1.1 }, glm::vec3{ 3, 6, 3 });
}

#ifndef CORNELL_DEBUG

void CornellBox::Draw(float aspect, float near, float far, Camera& camera,
    ShadowMap& shadowMap)
{
    static const char* name = "color";
    directShader_.Activate();
    directShader_.SetMat4("lightSpaceMat", shadowMap.GetLightSpaceMat());
    Texture::BindTextureOnShader(0, "shadowMap",
        directShader_, shadowMap.GetShadowBuffer());

    auto DrawSide = [&](const glm::vec3& color, BasicTriRenderModel& model)
    {
        directShader_.SetVec3(name, color);
        SetMVP(aspect, near, far, model, camera, directShader_);
        model.Draw(directShader_);
    };
    DrawSide({ 1,0,0 }, models_[0]);
    DrawSide({ 0,1,0 }, models_[1]);
    DrawSide({ 0,0,1 }, models_[2]);
    DrawSide({ 1,0,1 }, models_[3]);
    DrawSide({ 0,1,1 }, models_[4]);
    DrawSide({ 1,1,0 }, models_[5]);
    DrawSide({ 0.5,0.5,0.5 }, models_[6]);
    return;
}

#else

void CornellBox::Draw(float aspect, float near, float far, Camera& camera,
    ShadowMap& shadowMap)
{
    static const char* name = "color";
    directShader_.Activate();
    directShader_.SetMat4("lightSpaceMat", shadowMap.GetLightSpaceMat());
    Texture::BindTextureOnShader(0, "shadowMap",
        directShader_, shadowMap.GetShadowBuffer());

    auto DrawSide = [&](const glm::vec3& color, BasicTriRenderModel& model)
    {
        directShader_.SetVec3(name, color);
        SetMVP(aspect, near, far, model, camera, directShader_);
        model.Draw(directShader_);
    };
    DrawSide({ 1,0,0 }, models_[0]);
    DrawSide({ 0,1,0 }, models_[1]);

    // Other things are all white.
    directShader_.SetVec3(name, { 1,1,1 });
    for (size_t i = 2; i < models_.size(); i++)
    {
        SetMVP(aspect, near, far, models_[i], camera, directShader_);
        models_[i].Draw(directShader_);
    }
    return;
}

#endif

