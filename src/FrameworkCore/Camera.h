#pragma once
#ifndef OPENGLFRAMEWORK_CORE_CAMERA_H_
#define OPENGLFRAMEWORK_CORE_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>

namespace OpenGLFramework::Core
{

class Camera
{
public:
    Camera(const glm::vec3& init_pos, const glm::vec3& init_up, 
        const glm::vec3& init_front) : m_position(init_pos), m_gaze(
            glm::normalize(init_front)), m_up(glm::normalize(init_up - 
            m_gaze * glm::dot(m_gaze, init_up)) ) 
    {
        if (init_up == glm::zero<glm::vec3>() ||
            init_front == glm::zero<glm::vec3>()) [[unlikely]]
        {
            std::cout << "Detect zero vec in up or front for camera initialization,"
                "making up = (0, 1, 0) and gaze = (0, 0, -1)\n";
            m_up = { 0, 1, 0 };
            m_gaze = { 0, 0, -1 };
        }
    };

    float movementSpeed = 5.0f;
    float mouseSensitivity = 0.01f;
    float rotationSpeed = 30.0f;
    float fov = 45.0f;

    glm::vec3 Front() { return m_gaze; }
    glm::vec3 Back() { return -m_gaze; }
    glm::vec3 Up() { return m_up; }
    glm::vec3 Down() { return -m_up; }
    glm::vec3 Left() { return glm::cross(m_up, m_gaze); };
    glm::vec3 Right() { return glm::cross(m_gaze, m_up); };

    glm::mat4 GetViewMatrix() { 
        return glm::lookAt(m_position, m_position + m_gaze, m_up); };

    void Rotate(glm::vec3 eulerAngles) {
        glm::quat rotation = glm::quat(eulerAngles);
        Rotate(rotation);
        return;
    };

    inline void Rotate(glm::quat rotation)
    {
        m_gaze = rotation * m_gaze;
        m_up = rotation * m_up;
    }

    void Rotate(float angle, glm::vec3 axis)
    {
        glm::quat rotation = glm::angleAxis(glm::radians(angle), axis);
        Rotate(rotation);
    }

    void Translate(glm::vec3 vec)
    {
        m_position += vec;
        return;
    }

    glm::vec3 GetPosition() { return m_position; };
private:
    // Note that this sequence is deliberate, so that up will be initialized after front.
    glm::vec3 m_position;
    glm::vec3 m_gaze;
    glm::vec3 m_up;
};

}
#endif // !OPENGLFRAMEWORK_CORE_CAMERA_H_
