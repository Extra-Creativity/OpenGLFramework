#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace OpenGLFramework::Core
{

class Transform
{
public:
    glm::vec3 position = { 0, 0, 0 };
    glm::vec3 scale = { 1, 1, 1 };
    glm::quat rotation = { 1, 0, 0, 0 };

    glm::mat4 GetModelMatrix() const
    {
        auto result = glm::scale(glm::toMat4(rotation), scale);
        result[3] = { position, 1 };
        // Or scale * rot * translate(I, position), since these functions
        // right multiply, and this will get TRS correctly.
        return result;
    }

    Transform& Rotate(glm::vec3 eulerAngles) {
        glm::quat rotation = glm::quat(eulerAngles);
        return Rotate(rotation);
    };

    Transform& Rotate(glm::quat newRotation)
    {
        rotation = newRotation * rotation;
        return *this;
    }

    Transform& Rotate(float angle, glm::vec3 axis)
    {
        glm::quat rotation = glm::angleAxis(glm::radians(angle), axis);
        return Rotate(rotation);
    }

    Transform& Translate(glm::vec3 vec)
    {
        position += vec;
        return *this;
    }
};

} // namespace OpenGLFramework
