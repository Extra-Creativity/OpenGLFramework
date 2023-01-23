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
        glfwGetWindowSize(m_window, &width, &height);
        return { width, height };
    }

    template<int keyCode>
    void BindKeyPressed(UpdateFunc&& func)
    {
        static bool lastPress = true;
        m_keyPressedList.insert_or_assign(keyCode, [func, this]()
            {
                if (glfwGetKey(m_window, keyCode) == GLFW_PRESS)
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
        m_keyPressingList.insert_or_assign(keyCode, [func, this]()
            {
                if (glfwGetKey(m_window, keyCode) == GLFW_PRESS)
                    std::invoke(func);
            });
        return;
    }
    
    template<int keyCode>
    void BindKeyReleased(UpdateFunc&& func)
    {
        static bool lastRelease = true;
        m_keyReleasedList.insert_or_assign(keyCode, [func, this]()
            {
                if (glfwGetKey(m_window, keyCode) == GLFW_RELEASE)
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
        m_keyReleasingList.insert_or_assign(keyCode, [func, this]()
            {
                if (glfwGetKey(m_window, keyCode) == GLFW_RELEASE)
                    std::invoke(func);
            });
        return;
    };

    void BindScrollCallback(std::function<void(double, double)> callback);
    void BindCursorPosCallback(std::function<void(double, double)> callback);
    int GetKeyState(int keycode) { return glfwGetKey(m_window, keycode); }
    void SetInputMode(int mode, int value) { glfwSetInputMode(m_window, mode, value); }
    void Close() { glfwSetWindowShouldClose(m_window, true); }
    float deltaTime, currTime;

private:
    GLFWwindow* m_window;
    std::vector<UpdateFunc> m_routineList;
    std::unordered_map<int, UpdateFunc> m_keyPressedList;
    std::unordered_map<int, UpdateFunc> m_keyPressingList;
    std::unordered_map<int, UpdateFunc> m_keyReleasedList;
    std::unordered_map<int, UpdateFunc> m_keyReleasingList;
    std::unordered_map<int, UpdateFunc> m_mouseList;

    // here we do not set mutex, so multi-threading may be dangerous.
    //static std::unordered_map<GLFWwindow*, std::function<void(double, double)>> ms_scrollCallbacks;
    //static std::unordered_map<GLFWwindow*, std::function<void(double, double)>> ms_cursorPosCallbacks;
    //static std::unordered_map<GLFWwindow*, std::function<void(double, double)>> ms_mouseCallbacks;
 
    static std::function<void(double, double)> ms_scrollCallback;
    static std::function<void(double, double)> ms_cursorPosCallback;
    static std::function<void(double, double)> ms_mouseCallback;
    static bool m_singleton;
};

}
#endif // !OPENGLFRAMEWORK_MAINWINDOW_H_
