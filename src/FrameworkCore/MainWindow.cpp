#include "MainWindow.h"
#include "Utility/IO/IOExtension.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <limits>

namespace OpenGLFramework::Core
{

std::function<void(double, double)> MainWindow::s_scrollCallback_;

std::function<void(double, double)> MainWindow::s_cursorPosCallback_;

bool MainWindow::singletonFlag_ = true;

MainWindow::MainWindow() : deltaTime(0.0f), currTime(0.0f), window_(nullptr)
{}

MainWindow::MainWindow(unsigned int init_width, unsigned int init_height, 
    const char* title) : deltaTime(0.0f), currTime(0.0f), window_(nullptr)
{
    assert((init_width < std::numeric_limits<unsigned int>::max() 
        && init_height < std::numeric_limits<unsigned int>::max()));

    // note that this is not thread-safe.
    if (!singletonFlag_) [[unlikely]]
    {
        IOExtension::LogError("Try to use two GLFW windows simutanously.");
        abort();
    }
    singletonFlag_ = false;

    GLFWwindow* newWindow = glfwCreateWindow(static_cast<int>(init_width), 
        static_cast<int>(init_height), title, nullptr, nullptr);
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
    if (window_ != nullptr)
    {
        glfwDestroyWindow(window_);
        singletonFlag_ = true;
    }
    return;
};

MainWindow::MainWindow(MainWindow&& another) noexcept:
    window_{ another.window_ }, routineList_{std::move(routineList_)},
    pressedList_{ std::move(another.pressedList_) }, 
    pressingList_{ std::move(another.pressingList_) },
    releasedList_{std::move(another.releasedList_)},
    releasingList_{std::move(another.releasingList_)},
    deltaTime{ 0.0f }, currTime{0.0f}
{
    another.window_ = nullptr;
};

MainWindow& MainWindow::operator=(MainWindow&& another) noexcept
{
    assert(window_ == nullptr);

    window_ = another.window_;
    another.window_ = nullptr;
    routineList_ = std::move(routineList_);
    pressedList_ = std::move(another.pressedList_);
    pressingList_ = std::move(another.pressingList_);
    releasedList_ = std::move(another.releasedList_);
    releasingList_ = std::move(another.releasingList_);
    deltaTime = another.deltaTime;
    currTime = another.currTime;
    return *this;
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
        glEnable(GL_DEPTH_TEST);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        currRoutineID_ = 0;
        for (auto& func : routineList_)
        {
            std::invoke(func);
            currRoutineID_++;
        }

        for (auto& list : pressedList_)
        {
            for (auto& func : list)
            {
                std::invoke(func.second);
            }
        }

        for (auto& list : pressingList_)
        {
            for (auto& func : list)
            {
                std::invoke(func.second);
            }
        }

        for (auto& list : releasedList_)
        {
            for (auto& func : list)
            {
                std::invoke(func.second);
            }
        }

        for (auto& list : releasingList_)
        {
            for (auto& func : list)
            {
                std::invoke(func.second);
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
    return;
};

void MainWindow::Register(UpdateFunc&& func)
{
    routineList_.push_back(std::move(func));
}

void MainWindow::Register(UpdateFunc& func)
{
    routineList_.push_back(func);
};

size_t MainWindow::GetCurrRoutineID() { return currRoutineID_; };

void MainWindow::RemoveFromRoutines(size_t id)
{
    routineList_.erase(routineList_.begin() + id);
}

void MainWindow::BindScrollCallback(std::function<void(double, double)> callback)
{
    if (callback) // if callback is not empty.
    {
        s_scrollCallback_ = std::move(callback);
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
        s_cursorPosCallback_ = std::move(callback);
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
