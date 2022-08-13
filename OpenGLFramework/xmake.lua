set_project("OpenGLFramework")
set_version("1.0.0")
set_xmakever("2.6.1")

set_languages("cxx20")
if is_plat("linux") then
    set_config("cxx", "g++-11")
end

set_config("cuflags", "-std=c++17")

add_requires("glfw3", "glad", "glm", "assimp", "stb")
add_requires("imgui", {configs={glfw_opengl3 = true}})
add_packages("glfw3", "glad", "glm", "assimp", "imgui", "stb")

target("main")
    -- get executable file.
    set_kind("binary")

    -- set global config to use.
    set_configvar("OPENGLFRAMEWORK_ENABLE_GPUEXTENSION", 0)
    -- if your GPU is quite fast , replace code above with :
    -- set_configvar("OPENGLFRAMEWORK_ENABLE_GPUEXTENSION", 1)
    -- add_files("src/FrameCore/cu/*.cu")

    set_configdir("src/FrameCore/config")
    add_configfiles("src/FrameCore/config/config.h.in")
    set_configvar("OPENGLFRAMEWORK_RESOURCE_DIR", "$(projectdir)/Resources/")
    set_configvar("OPENGLFRAMEWORK_SHADER_DIR", "$(projectdir)/src/Shaders/")

    -- determine files and paths.
    add_includedirs("src/FrameCore/headers")
    add_includedirs("src/FrameCore/config")
    add_files("src/FrameCore/cpp/*.cpp")
    add_files("src/main.cpp")
