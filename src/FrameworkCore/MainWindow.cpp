#include "MainWindow.h"
#include "Utility/IO/IOExtension.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

//std::unordered_map<GLFWwindow*, std::function<void(double, double)>> 
//    MainWindow::ms_scrollCallbacks;
//
//std::unordered_map<GLFWwindow*, std::function<void(double, double)>> 
//    MainWindow::ms_cursorPosCallbacks;

namespace OpenGLFramework::Core
{

std::function<void(double, double)> MainWindow::ms_scrollCallback;

std::function<void(double, double)> MainWindow::ms_cursorPosCallback;

bool MainWindow::m_singleton = true;

MainWindow::MainWindow(int init_width, int init_height, const char* title) :
    deltaTime(0.0f), currTime(0.0f), m_window(nullptr)
{
    // note that this is not thread-safe.
    if (!m_singleton) [[unlikely]]
    {
        IOExtension::LogError("Try to use two GLFW windows simutanously.");
        abort();
    }
    m_singleton = false;

    GLFWwindow* newWindow = glfwCreateWindow(init_width, init_height, title, nullptr, nullptr);
    if (newWindow == nullptr) [[unlikely]]
    { 
        IOExtension::LogError("Fail to create GLFW window.");
        return;
    }
    glfwMakeContextCurrent(newWindow);
    //glfwSetFramebufferSizeCallback(newWindow,
    //    [](GLFWwindow* window, int m_width, int m_height)
    //    {
    //        glViewport(0, 0, m_width, m_height);
    //        return;
    //    }
    //);
    if(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) [[unlikely]]
        IOExtension::LogError("Fail to initialize GLAD.");
    if(!ImGui_ImplOpenGL3_Init("#version 330")) [[unlikely]]
        IOExtension::LogError("Fail to initialize ImGui for OpenGL3.");
    if(!ImGui_ImplGlfw_InitForOpenGL(newWindow, true)) [[unlikely]]
        IOExtension::LogError("Fail to initialize ImGui for GLFW.");

    m_window = newWindow;
    return;
};

MainWindow::~MainWindow()
{
    glfwDestroyWindow(m_window);
    m_singleton = true;
    return;
};

void MainWindow::MainLoop(const glm::vec4& backgroundColor)
{
    while (!glfwWindowShouldClose(m_window))
    {
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        deltaTime = static_cast<float>(glfwGetTime()) - currTime;
        currTime += deltaTime;

        const auto [m_width, m_height] = GetWidthAndHeight();
        glViewport(0, 0, m_width, m_height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for (auto& func : m_routineList)
        {
            std::invoke(func);
        }
        for (auto& func : m_keyPressedList)
        {
            std::invoke(func.second);
        }

        for (auto& func : m_keyPressingList)
        {
            std::invoke(func.second);
        }

        for (auto& func : m_keyReleasedList)
        {
            std::invoke(func.second);
        }

        for (auto& func : m_keyReleasingList)
        {
            std::invoke(func.second);
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    return;
};

int MainWindow::Register(UpdateFunc&& func)
{
    m_routineList.push_back(std::forward<UpdateFunc>(func));
    return static_cast<int>(m_routineList.size()) - 1;
}

int MainWindow::Register(UpdateFunc& func)
{
    m_routineList.push_back(func);
    return static_cast<int>(m_routineList.size()) - 1;
};

void MainWindow::BindScrollCallback(std::function<void(double, double)> callback)
{
    //if (callback)
    //{
    //    ms_scrollCallbacks.insert_or_assign(m_window, callback);
    //    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
    //        auto pos = ms_scrollCallbacks.find(window);
    //        pos->second(xOffset, yOffset);
    //        });
    //}
    //else
    //{
    //    ms_scrollCallbacks.erase(m_window);
    //    glfwSetScrollCallback(m_window, nullptr);
    //}

    if (callback) // if callback is not empty.
    {
        ms_scrollCallback = callback;
        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            ms_scrollCallback(xOffset, yOffset);
        });
    }
    else
    {
        ms_scrollCallback = nullptr;
        glfwSetScrollCallback(m_window, nullptr);
    }
    return;
}

void MainWindow::BindCursorPosCallback(std::function<void(double, double)> callback)
{
    //if (callback)
    //{
    //    ms_cursorPosCallbacks.insert_or_assign(m_window, callback);
    //    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
    //        auto pos = ms_cursorPosCallbacks.find(window);
    //        pos->second(xOffset, yOffset);
    //        });
    //}
    //else
    //{
    //    ms_cursorPosCallbacks.erase(m_window);
    //    glfwSetScrollCallback(m_window, nullptr);
    //}

    if (callback) // if callback is not empty.
    {
        ms_cursorPosCallback = callback;
        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            ms_cursorPosCallback(xOffset, yOffset);
        });
    }
    else
    {
        ms_cursorPosCallback = nullptr;
        glfwSetCursorPosCallback(m_window, nullptr);
    }
    return;
}

} // namespace OpenGLFramework::Core
