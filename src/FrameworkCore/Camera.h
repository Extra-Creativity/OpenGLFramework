#pragma once
#ifndef OPENGLFRAMEWORK_CORE_CAMERA_H_
#define OPENGLFRAMEWORK_CORE_CAMERA_H_

#include "../Utility/IO/IOExtension.h"

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
    float mouseSensitivity = 0.1f;
    float rotationSpeed = 30.0f;
    float fov = 45.0f;

    glm::vec3 Front() const { return gaze_; }
    glm::vec3 Back() const { return -gaze_; }
    glm::vec3 Up() const { return up_; }
    glm::vec3 Down() const { return -up_; }
    glm::vec3 Left() const { return glm::cross(up_, gaze_); };
    glm::vec3 Right() const { return glm::cross(gaze_, up_); };

    glm::mat4 GetViewMatrix() const { 
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
        glm::quat rotation = 
            glm::angleAxis(glm::radians(angle), glm::normalize(axis));
        Rotate(rotation);
    }

    void Translate(glm::vec3 vec)
    {
        position_ += vec;
        return;
    }

    void RotateAroundCenter(float angle, glm::vec3 axis, const glm::vec3& center)
    {
        glm::vec3 distanceVec = position_ - center;
        glm::quat rotation =
            glm::angleAxis(glm::radians(angle), glm::normalize(axis));
        Rotate(rotation);
        position_ = rotation * distanceVec + center;
        return;
    }

    glm::vec3 GetPosition() const { return position_; };
private:
    // Note that this sequence is deliberate, so that up will be initialized after front.
    glm::vec3 position_;
    glm::vec3 gaze_;
    glm::vec3 up_;
};

}
#endif // !OPENGLFRAMEWORK_CORE_CAMERA_H_
