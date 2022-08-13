#pragma once
#ifndef _OPENGLFRAMEWORK_TRANSFORM_H_
#define _OPENGLFRAMEWORK_TRANSFORM_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

class Transform
{
public:
    glm::vec3 position = {0, 0, 0};
    glm::vec3 scale = {1, 1, 1};
    glm::quat rotation = {1, 0, 0, 0};

    glm::mat4 GetModelMatrix()
    {
        return glm::translate(glm::scale(glm::toMat4(rotation), scale), position);
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


#endif // !_OPENGLFRAMEWORK_TRANSFORM_H_
