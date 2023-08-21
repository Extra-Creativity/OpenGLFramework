#pragma once
#ifndef OPENGLFRAMEWORK_MAINWINDOW_H_
#define OPENGLFRAMEWORK_MAINWINDOW_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <functional>
#include <vector>
#include <unordered_map>
#include <array>
#include <filesystem>

namespace OpenGLFramework::Core
{

class MainWindow
{
    using UpdateFunc = std::function<void(void)>;
    constexpr static int handleAmount_ = 2;
    enum class InputHandleBindType { Key = 0, MouseButton = 1 };
    using InputHandleFunc = decltype(glfwGetKey);
    using InputToUpdateFuncMap = std::unordered_map<int, UpdateFunc>;
public:
    MainWindow();
    MainWindow(unsigned int m_height, unsigned int m_width, const char* title,
        bool visible = true);
    ~MainWindow();
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) noexcept;
    MainWindow& operator=(MainWindow&&) noexcept;

    void MainLoop(const glm::vec4& backgroundColor);
    void Register(UpdateFunc&& func);
    void Register(UpdateFunc& func);
    void ClearRoutines();
    void BindScrollCallback(std::function<void(double, double)> callback);
    void BindCursorPosCallback(std::function<void(double, double)> callback);
private:
    template<int keyCode, InputHandleBindType type>
    void BindPressed_(UpdateFunc&& func, InputHandleFunc handleFunc)
    {
        auto& currList = pressedList_[static_cast<int>(type)];
        if (func == nullptr) [[unlikely]]
        { 
            currList.erase(keyCode);
            return;
        }

        static bool lastPress = true;
        currList.insert_or_assign(keyCode,
            [func, handleFunc, this]() {
                if (handleFunc(window_, keyCode) == GLFW_PRESS)
                {
                    if (!lastPress)
                    {
                        lastPress = true;
                        std::invoke(func);
                    }
                }
                else // Release.
                    lastPress = false;
            });
        return;
    }

    template<int keyCode, InputHandleBindType type>
    void BindPressing_(UpdateFunc&& func, InputHandleFunc handleFunc)
    {
        auto& currList = pressingList_[static_cast<int>(type)];
        if (func == nullptr) [[unlikely]]
        {
            currList.erase(keyCode);
            return;
        }

        currList.insert_or_assign(keyCode,
            [func, handleFunc, this]() {
                if (handleFunc(window_, keyCode) == GLFW_PRESS)
                    std::invoke(func);
            });
        return;
    }

    template<int keyCode, InputHandleBindType type>
    void BindReleased_(UpdateFunc&& func, InputHandleFunc handleFunc)
    {
        auto& currList = releasedList_[static_cast<int>(type)];
        if (func == nullptr) [[unlikely]]
        {
            currList.erase(keyCode);
            return;
        }

        static bool lastRelease = true;
        currList.insert_or_assign(keyCode,
            [func, handleFunc, this]() {
                if (handleFunc(window_, keyCode) == GLFW_RELEASE)
                {
                    if (!lastRelease)
                    {
                        lastRelease = true;
                        std::invoke(func);
                    }
                }
                else // Press.
                    lastRelease = false;
            });

        return;
    };

    template<int keyCode, InputHandleBindType type>
    void BindReleasing_(UpdateFunc&& func, InputHandleFunc handleFunc)
    {
        auto& currList = releasingList_[static_cast<int>(type)];
        if (func == nullptr) [[unlikely]]
        {
            currList.erase(keyCode);
            return;
        }

        currList.insert_or_assign(keyCode,
            [func, handleFunc, this]() {
                if (handleFunc(window_, keyCode) == GLFW_RELEASE)
                    std::invoke(func);
            });
        return;
    };

    std::vector<unsigned char> GetPixelsFromGPU_(int, int, int) const;
public:
    template<int keyCode>
    void BindKeyPressed(UpdateFunc&& func)
    {
        BindPressed_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int keyCode>
    void BindKeyPressing(UpdateFunc&& func)
    {
        BindPressing_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int keyCode>
    void BindKeyReleased(UpdateFunc&& func)
    {
        BindReleased_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int keyCode>
    void BindKeyReleasing(UpdateFunc&& func)
    {
        BindReleasing_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int mouseButtonCode>
    void BindMouseButtonPressed(UpdateFunc&& func)
    {
        BindPressed_<mouseButtonCode, InputHandleBindType::MouseButton>(
            std::move(func), glfwGetMouseButton);
    }

    template<int mouseButtonCode>
    void BindMouseButtonPressing(UpdateFunc&& func)
    {
        BindPressing_<mouseButtonCode, InputHandleBindType::MouseButton>(
            std::move(func), glfwGetMouseButton);
    }

    template<int mouseButtonCode>
    void BindMouseButtonReleased(UpdateFunc&& func)
    {
        BindReleased_<mouseButtonCode, InputHandleBindType::MouseButton>(
            std::move(func), glfwGetMouseButton);
    }

    template<int mouseButtonCode>
    void BindMouseButtonReleasing(UpdateFunc&& func)
    {
        BindReleasing_<mouseButtonCode, InputHandleBindType::MouseButton>(
            std::move(func), glfwGetMouseButton);
    }

    std::pair<unsigned int, unsigned int> GetWidthAndHeight()
    {
        int width, height;
        glfwGetWindowSize(window_, &width, &height);
        return { width, height };
    }

    std::pair<float, float> GetCursorPos()
    {
        double xPos, yPos;
        glfwGetCursorPos(window_, &xPos, &yPos);
        return { static_cast<float>(xPos), static_cast<float>(yPos) };
    }

    int GetKeyState(int keycode) { return glfwGetKey(window_, keycode); }
    void SetInputMode(int mode, int value) const {
        glfwSetInputMode(window_, mode, value);
    }
    void Hide(bool hide = true) const { 
        if (hide)
            glfwHideWindow(window_);
        else
            glfwShowWindow(window_);
    }
    void Close() const { glfwSetWindowShouldClose(window_, true); }
    void SaveImage(const std::filesystem::path& path, bool needFlip = true) const;
    GLFWwindow* GetNativeHandler() const { return window_; }
    float GetDeltaTime() const { return deltaTime_; }
    float GetCurrTime() const { return currTime_; }
private:
    float deltaTime_, currTime_;
    GLFWwindow* window_;
    size_t currRoutineID_ = 0;
    std::vector<UpdateFunc> routineList_;
    std::array<InputToUpdateFuncMap, handleAmount_> pressedList_;
    std::array<InputToUpdateFuncMap, handleAmount_> pressingList_;
    std::array<InputToUpdateFuncMap, handleAmount_> releasedList_;
    std::array<InputToUpdateFuncMap, handleAmount_> releasingList_;
 
    static std::function<void(double, double)> s_scrollCallback_;
    static std::function<void(double, double)> s_cursorPosCallback_;
    static std::function<void(double, double)> s_mouseCallback_;
    static bool singletonFlag_;
};

}
#endif // !OPENGLFRAMEWORK_MAINWINDOW_H_
