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
        wall.transform.Rotate(degree, axis);
        wall.transform.Translate(translation);
        wall.transform.scale = glm::vec3{ 5 };
        models.push_back(std::move(wall));
    };

    InsertNewQuad(90, { 0, 1, 0 }, { -5, 0, 0 }); // left wall
    InsertNewQuad(90, { 0, 1, 0 }, { 5, 0, 0 }); // right wall
    InsertNewQuad(90, { 1, 0, 0 }, { 0, 5, 0 }); // ceil
    InsertNewQuad(90, { -1, 0, 0 }, { 0, -5, 0 }); // floor
    InsertNewQuad(0, { 0, 1, 0 }, { 0, 0, -5 }); // back
}

void CornellBox::Draw(float aspect, float near, float far, Camera& camera)
{
    static const char* name = "color";
    directShader_.Activate();
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