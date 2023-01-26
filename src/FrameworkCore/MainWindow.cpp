#include "MainWindow.h"
#include "Utility/IO/IOExtension.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace OpenGLFramework::Core
{

std::function<void(double, double)> MainWindow::s_scrollCallback_;

std::function<void(double, double)> MainWindow::s_cursorPosCallback_;

bool MainWindow::singletonFlag_ = true;

MainWindow::MainWindow(int init_width, int init_height, const char* title) :
    deltaTime(0.0f), currTime(0.0f), window_(nullptr)
{
    // note that this is not thread-safe.
    if (!singletonFlag_) [[unlikely]]
    {
        IOExtension::LogError("Try to use two GLFW windows simutanously.");
        abort();
    }
    singletonFlag_ = false;

    GLFWwindow* newWindow = glfwCreateWindow(init_width, init_height, title,
        nullptr, nullptr);
    if (newWindow == nullptr) [[unlikely]]
    { 
        IOExtension::LogError("Fail to create GLFW window.");
        return;
    }
    glfwMakeContextCurrent(newWindow);
    if(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) 
        [[unlikely]]
        IOExtension::LogError("Fail to initialize GLAD.");
    if(!ImGui_ImplOpenGL3_Init("#version 330")) [[unlikely]]
        IOExtension::LogError("Fail to initialize ImGui for OpenGL3.");
    if(!ImGui_ImplGlfw_InitForOpenGL(newWindow, true)) [[unlikely]]
        IOExtension::LogError("Fail to initialize ImGui for GLFW.");

    window_ = newWindow;
    return;
};

MainWindow::~MainWindow()
{
    glfwDestroyWindow(window_);
    singletonFlag_ = true;
    return;
};

void MainWindow::MainLoop(const glm::vec4& backgroundColor)
{
    while (!glfwWindowShouldClose(window_))
    {
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 
            backgroundColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        deltaTime = static_cast<float>(glfwGetTime()) - currTime;
        currTime += deltaTime;

        const auto [m_width, m_height] = GetWidthAndHeight();
        glViewport(0, 0, m_width, m_height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for (auto& func : routineList_)
        {
            std::invoke(func);
        }
        for (auto& func : keyPressedList_)
        {
            std::invoke(func.second);
        }

        for (auto& func : keyPressingList_)
        {
            std::invoke(func.second);
        }

        for (auto& func : keyReleasedList_)
        {
            std::invoke(func.second);
        }

        for (auto& func : keyReleasingList_)
        {
            std::invoke(func.second);
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
    return;
};

int MainWindow::Register(UpdateFunc&& func)
{
    routineList_.push_back(std::forward<UpdateFunc>(func));
    return static_cast<int>(routineList_.size()) - 1;
}

int MainWindow::Register(UpdateFunc& func)
{
    routineList_.push_back(func);
    return static_cast<int>(routineList_.size()) - 1;
};

void MainWindow::BindScrollCallback(std::function<void(double, double)> callback)
{
    if (callback) // if callback is not empty.
    {
        s_scrollCallback_ = callback;
        glfwSetScrollCallback(window_, 
            [](GLFWwindow* window, double xOffset, double yOffset) {
                s_scrollCallback_(xOffset, yOffset);
            });
    }
    else
    {
        s_scrollCallback_ = nullptr;
        glfwSetScrollCallback(window_, nullptr);
    }
    return;
}

void MainWindow::BindCursorPosCallback(std::function<void(double, double)> callback)
{
    if (callback) // if callback is not empty.
    {
        s_cursorPosCallback_ = callback;
        glfwSetCursorPosCallback(window_, 
            [](GLFWwindow* window, double xOffset, double yOffset) {
                s_cursorPosCallback_(xOffset, yOffset);
            });
    }
    else
    {
        s_cursorPosCallback_ = nullptr;
        glfwSetCursorPosCallback(window_, nullptr);
    }
    return;
}

} // namespace OpenGLFramework::Core
