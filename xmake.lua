set_project("OpenGLFramework")
set_version("1.0.0")
set_xmakever("2.6.1")

set_languages("cxx20")
if is_plat("linux") then
    set_config("cxx", "g++-11")
end

set_config("cuflags", "-std=c++17")

-- If you have vcpkg libs, you can change them with vcpkg::...
-- for imgui, you should add imgui[opengl3] and imgui[glfw].
add_requires("glfw", "glad", "glm", "assimp", "stb")
add_requires("imgui", {configs={glfw_opengl3 = true}})
add_packages("glfw", "glad", "glm", "assimp", "imgui", "stb")

target("main")
    -- get executable file.
    set_kind("binary")

    --set config directory.
    set_configdir("src/FrameCore/config")
    add_configfiles("src/FrameCore/config/config.h.in")

    -- set global config to use.
    set_configvar("OPENGLFRAMEWORK_ENABLE_GPUEXTENSION", 0)
    -- if your GPU is quite fast , replace code above with :
    -- set_configvar("OPENGLFRAMEWORK_ENABLE_GPUEXTENSION", 1)
    -- add_files("src/FrameCore/cu/*.cu")
    
    -- set resource and shader directory config.
    dir, _ = path.join(os.projectdir(), "Resources"):gsub("\\", "/")
    set_configvar("OPENGLFRAMEWORK_RESOURCES_DIR", dir)
    
    dir, _ = path.join(os.projectdir(), "src", "Shaders"):gsub("\\", "/") 
    set_configvar("OPENGLFRAMEWORK_SHADERS_DIR", dir)

    -- determine files and paths.
    add_includedirs("src/FrameCore/headers")
    add_includedirs("src/FrameCore/config")
    add_files("src/FrameCore/cpp/*.cpp")
    add_files("src/main.cpp")