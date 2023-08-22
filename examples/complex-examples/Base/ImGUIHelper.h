#pragma once
#include "Utility/Generator/Generator.h"
#include "FrameworkCore/MainWindow.h"

#include <string>
#include <functional>
#include <imgui.h>

namespace ExampleBase
{

inline float s_ImGuiHelper_y = 25.0f;
inline const float s_ImGuiHelper_interval = 25.0f;

template<typename T>
class ImGuiHelper
{
private:
    using Handle = void(*)(T&);
public:
    ImGuiHelper(unsigned int init_width, unsigned int init_height, 
        std::string init_name, T init_data, Handle func) : 
        width_{ static_cast<float>(init_width) }, 
        height_{ static_cast<float>(init_height) },
        name_{ std::move(init_name) }, data_{ std::move(init_data) },
        savedContext_{ Work(func) }, contextIter_{ savedContext_.begin() }
    { };

    ImGuiHelper(unsigned int init_width, unsigned int init_height,
        std::string init_name, T init_data, Handle func, Handle func2) :
        width_{ static_cast<float>(init_width) },
        height_{ static_cast<float>(init_height) },
        name_{ std::move(init_name) }, data_{ std::move(init_data) },
        savedContext_{ Work(func, func2) },
        contextIter_{ savedContext_.begin() }
    { };

    OpenGLFramework::Coroutine::Generator<int> Work(Handle work)
    {
        float currY = AllocateVerticalPosition_();
        co_yield 0;
        ImGui::Begin(name_.c_str());
        ImGui::SetWindowPos({ 50, currY });
        ImGui::SetWindowSize({ width_, height_ });

        while (true) {
            work(data_);
            ImGui::End();
            co_yield 0;
            ImGui::Begin(name_.c_str());
        }
    }

    OpenGLFramework::Coroutine::Generator<int> Work(Handle work,
        Handle restart)
    {
        float currY = AllocateVerticalPosition_();
        co_yield 0;
        ImGui::Begin(name_.c_str());
        ImGui::SetWindowPos({ 50, currY });
        ImGui::SetWindowSize({ width_, height_ });

        while (true) {
            work(data_);
            ImGui::End();
            co_yield 0;
            restart(data_);
            ImGui::Begin(name_.c_str());
        }
    }

    T& GetData() { return data_; }
    void RegisterOnMainWindow(OpenGLFramework::Core::MainWindow& mainWindow) {
        mainWindow.Register([&iter = contextIter_]() { iter++; });
    }
private:
    float AllocateVerticalPosition_() {
        float currY = s_ImGuiHelper_y;
        s_ImGuiHelper_y += height_ + s_ImGuiHelper_interval;
        return currY;
    }

    float width_;
    float height_;
    std::string name_;

    T data_;
    OpenGLFramework::Coroutine::Generator<int> savedContext_;
    typename decltype(savedContext_)::Iter contextIter_;
};

}