#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
        auto result = glm::scale(glm::mat4_cast(rotation), scale);
        result[3] = { position, 1 };
        // Or scale * rot * translate(I, position), since these functions
        // right multiply, and this will get TRS correctly.
        return result;
    }

    void Rotate(glm::vec3 eulerAngles) {
        glm::quat rotation = glm::quat(eulerAngles);
        Rotate(rotation);
        return;
    };

    void Rotate(glm::quat newRotation)
    {
        rotation = newRotation * rotation;
    }

    void Rotate(float angle, glm::vec3 axis)
    {
        glm::quat rotation = glm::angleAxis(angle, axis);
        Rotate(rotation);
    }

    void Translate(glm::vec3 vec)
    {
        position += vec;
        return;
    }
};

} // namespace OpenGLFramework
