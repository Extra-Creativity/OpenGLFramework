#include "ContextManager.h"
#include "Utility/IO/IOExtension.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace OpenGLFramework::Core
{

using namespace OpenGLFramework;

ContextManager& ContextManager::GetInstance()
{
    static ContextManager manager{};
    return manager;
}

ContextManager::ContextManager()
{
    InitGLFWContext_();
    InitImGuiContext_();
}

ContextManager::~ContextManager()
{
    EndAllContext_();
}

void ContextManager::InitGLFWContext_()
{   
    auto initResult = glfwInit();
    if (initResult == GLFW_FALSE) [[unlikely]]
        IOExtension::LogError("Fail to initialize GLFW.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    return;
};

void ContextManager::InitImGuiContext_()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    [[maybe_unused]] ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsClassic();
    ImVec4 clear_color{ 0.45f, 0.55f, 0.60f, 1.00f };

    io.IniFilename = NULL;
    return;
}

void ContextManager::EndAllContext_()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return;
}

} // namespace OpenGLFramework::Core.