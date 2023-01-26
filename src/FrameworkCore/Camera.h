#pragma once
#ifndef OPENGLFRAMEWORK_CORE_CAMERA_H_
#define OPENGLFRAMEWORK_CORE_CAMERA_H_

#include "Utility/IO/IOExtension.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace OpenGLFramework::Core
{

class Camera
{
public:
    Camera(const glm::vec3& init_pos, const glm::vec3& init_up, 
        const glm::vec3& init_front) : position_(init_pos), gaze_(
            glm::normalize(init_front)), up_(glm::normalize(init_up - 
            gaze_ * glm::dot(gaze_, init_up)) ) 
    {
        if (init_up == glm::zero<glm::vec3>() ||
            init_front == glm::zero<glm::vec3>()) [[unlikely]]
        {
            OpenGLFramework::IOExtension::LogError(
                "Detect zero vec in up or front for camera initialization,"
                "making up = (0, 1, 0) and gaze = (0, 0, -1)\n");
            up_ = { 0, 1, 0 };
            gaze_ = { 0, 0, -1 };
        }
    };

    float movementSpeed = 5.0f;
    float mouseSensitivity = 0.01f;
    float rotationSpeed = 30.0f;
    float fov = 45.0f;

    glm::vec3 Front() { return gaze_; }
    glm::vec3 Back() { return -gaze_; }
    glm::vec3 Up() { return up_; }
    glm::vec3 Down() { return -up_; }
    glm::vec3 Left() { return glm::cross(up_, gaze_); };
    glm::vec3 Right() { return glm::cross(gaze_, up_); };

    glm::mat4 GetViewMatrix() { 
        return glm::lookAt(position_, position_ + gaze_, up_); };

    void Rotate(glm::vec3 eulerAngles) {
        glm::quat rotation = glm::quat(eulerAngles);
        Rotate(rotation);
        return;
    };

    inline void Rotate(glm::quat rotation)
    {
        gaze_ = rotation * gaze_;
        up_ = rotation * up_;
    }

    void Rotate(float angle, glm::vec3 axis)
    {
        glm::quat rotation = glm::angleAxis(glm::radians(angle), axis);
        Rotate(rotation);
    }

    void Translate(glm::vec3 vec)
    {
        position_ += vec;
        return;
    }

    glm::vec3 GetPosition() { return position_; };
private:
    // Note that this sequence is deliberate, so that up will be initialized after front.
    glm::vec3 position_;
    glm::vec3 gaze_;
    glm::vec3 up_;
};

}
#endif // !OPENGLFRAMEWORK_CORE_CAMERA_H_
