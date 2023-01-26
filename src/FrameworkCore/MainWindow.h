#pragma once
#ifndef OPENGLFRAMEWORK_MAINWINDOW_H_
#define OPENGLFRAMEWORK_MAINWINDOW_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <functional>
#include <vector>
#include <unordered_map>

namespace OpenGLFramework::Core
{

class MainWindow
{
    using UpdateFunc = std::function<void(void)>;
public:
    MainWindow(int m_height, int m_width, const char* title);
    ~MainWindow();
    MainWindow(MainWindow&) = delete;
    void MainLoop(const glm::vec4& backgroundColor);
    int Register(UpdateFunc&& func);
    int Register(UpdateFunc& func);

    std::pair<unsigned int, unsigned int> GetWidthAndHeight()
    {
        int width, height;
        glfwGetWindowSize(window_, &width, &height);
        return { width, height };
    }

    template<int keyCode>
    void BindKeyPressed(UpdateFunc&& func)
    {
        static bool lastPress = true;
        keyPressedList_.insert_or_assign(keyCode, 
            [func, this]() {
                if (glfwGetKey(window_, keyCode) == GLFW_PRESS)
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
    
    template<int keyCode>
    void BindKeyPressing(UpdateFunc&& func)
    {
        keyPressingList_.insert_or_assign(keyCode, 
            [func, this]() {
                if (glfwGetKey(window_, keyCode) == GLFW_PRESS)
                    std::invoke(func);
            });
        return;
    }
    
    template<int keyCode>
    void BindKeyReleased(UpdateFunc&& func)
    {
        static bool lastRelease = true;
        keyReleasedList_.insert_or_assign(keyCode, 
            [func, this](){
                if (glfwGetKey(window_, keyCode) == GLFW_RELEASE)
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
    
    template<int keyCode>
    void BindKeyReleasing(UpdateFunc&& func)
    {
        keyReleasingList_.insert_or_assign(keyCode, 
            [func, this]() {
                if (glfwGetKey(window_, keyCode) == GLFW_RELEASE)
                    std::invoke(func);
            });
        return;
    };

    void BindScrollCallback(std::function<void(double, double)> callback);
    void BindCursorPosCallback(std::function<void(double, double)> callback);
    int GetKeyState(int keycode) { return glfwGetKey(window_, keycode); }
    void SetInputMode(int mode, int value) { 
        glfwSetInputMode(window_, mode, value); 
    }
    void Close() { glfwSetWindowShouldClose(window_, true); }
    float deltaTime, currTime;

private:
    GLFWwindow* window_;
    std::vector<UpdateFunc> routineList_;
    std::unordered_map<int, UpdateFunc> keyPressedList_;
    std::unordered_map<int, UpdateFunc> keyPressingList_;
    std::unordered_map<int, UpdateFunc> keyReleasedList_;
    std::unordered_map<int, UpdateFunc> keyReleasingList_;
    std::unordered_map<int, UpdateFunc> mouseList_;
 
    static std::function<void(double, double)> s_scrollCallback_;
    static std::function<void(double, double)> s_cursorPosCallback_;
    static std::function<void(double, double)> s_mouseCallback_;
    static bool singletonFlag_;
};

}
#endif // !OPENGLFRAMEWORK_MAINWINDOW_H_
