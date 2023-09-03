#pragma once

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

class IRoutineFactory
{
public:
    using Routine = std::function<void(void)>;
    using RoutineContainer = std::vector<Routine>;
    bool& ShouldCreate() { return signal_; };
    virtual RoutineContainer CreateRoutines() { return {}; };
private:
    bool signal_ = false;
};

class MainWindow
{
    constexpr static int handleAmount_ = 2;
    enum class InputHandleBindType { Key = 0, MouseButton = 1 };
    using InputHandleFunc = decltype(glfwGetKey);
    using InputToRoutineMap = std::unordered_map<int, IRoutineFactory::Routine>;
public:
    MainWindow();
    MainWindow(unsigned int m_height, unsigned int m_width, const char* title,
        bool visible = true, std::unique_ptr<IRoutineFactory> factory = 
            std::make_unique<IRoutineFactory>());
    ~MainWindow();
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) noexcept;
    MainWindow& operator=(MainWindow&&) noexcept;

    void MainLoop(const glm::vec4& backgroundColor);
    void Register(IRoutineFactory::Routine func);
    void ClearRoutines();
    void BindScrollCallback(std::function<void(double, double)> callback);
    void BindCursorPosCallback(std::function<void(double, double)> callback);
private:
    template<int keyCode, InputHandleBindType type>
    void BindPressed_(IRoutineFactory::Routine&& func, InputHandleFunc handleFunc)
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
    void BindPressing_(IRoutineFactory::Routine&& func, InputHandleFunc handleFunc)
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
    void BindReleased_(IRoutineFactory::Routine&& func, InputHandleFunc handleFunc)
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
    void BindReleasing_(IRoutineFactory::Routine&& func, InputHandleFunc handleFunc)
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
    void BindKeyPressed(IRoutineFactory::Routine func)
    {
        BindPressed_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int keyCode>
    void BindKeyPressing(IRoutineFactory::Routine func)
    {
        BindPressing_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int keyCode>
    void BindKeyReleased(IRoutineFactory::Routine func)
    {
        BindReleased_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int keyCode>
    void BindKeyReleasing(IRoutineFactory::Routine func)
    {
        BindReleasing_<keyCode, InputHandleBindType::Key>(std::move(func), glfwGetKey);
    }

    template<int mouseButtonCode>
    void BindMouseButtonPressed(IRoutineFactory::Routine func)
    {
        BindPressed_<mouseButtonCode, InputHandleBindType::MouseButton>(
            std::move(func), glfwGetMouseButton);
    }

    template<int mouseButtonCode>
    void BindMouseButtonPressing(IRoutineFactory::Routine func)
    {
        BindPressing_<mouseButtonCode, InputHandleBindType::MouseButton>(
            std::move(func), glfwGetMouseButton);
    }

    template<int mouseButtonCode>
    void BindMouseButtonReleased(IRoutineFactory::Routine func)
    {
        BindReleased_<mouseButtonCode, InputHandleBindType::MouseButton>(
            std::move(func), glfwGetMouseButton);
    }

    template<int mouseButtonCode>
    void BindMouseButtonReleasing(IRoutineFactory::Routine func)
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
    
    float GetAspect() { 
        auto [width, height] = GetWidthAndHeight();
        return static_cast<float>(width) / height;
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
    void SetRoutineFactory(std::unique_ptr<IRoutineFactory> init) { 
        factory_ = std::move(init);
    }
private:
    float deltaTime_, currTime_;
    GLFWwindow* window_;
    
    std::unique_ptr<IRoutineFactory> factory_;
    std::vector<IRoutineFactory::Routine> routineList_;
    std::array<InputToRoutineMap, handleAmount_> pressedList_;
    std::array<InputToRoutineMap, handleAmount_> pressingList_;
    std::array<InputToRoutineMap, handleAmount_> releasedList_;
    std::array<InputToRoutineMap, handleAmount_> releasingList_;
 
    static std::function<void(double, double)> s_scrollCallback_;
    static std::function<void(double, double)> s_cursorPosCallback_;
    static std::function<void(double, double)> s_mouseCallback_;
    static bool singletonFlag_;
};

}
