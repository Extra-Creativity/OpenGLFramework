target("ExampleBase")
    set_kind("static")
    add_deps("OpenGLFramework")
    add_includedirs("../../src/")
    add_files("Base/*.cpp")
    add_headerfiles("Base/*.h")

target("SoftShadow-Advanced")
    set_kind("binary")
    config_dir = path.join(os.curdir(), "SoftShadow-Advanced", "Config"):gsub("\\", "/")
    shader_dir = path.join(os.curdir(), "SoftShadow-Advanced", "Shaders"):gsub("\\", "/")
    model_dir = path.join(os.projectdir(), "Resources", "Models"):gsub("\\", "/")

    add_defines(
        "CONFIG_DIR=\"" .. config_dir .. "\"", 
        "SHADER_DIR=\"" .. shader_dir .. "\"", 
        "MODEL_DIR=\"" .. model_dir .. "\"")
    add_includedirs("../../src/")
    add_deps("ExampleBase")
    add_files("SoftShadow-Advanced/*.cpp")
    add_headerfiles("SoftShadow-Advanced/*.h")

target("Scene-Switch")
    set_kind("binary")
    config_dir = path.join(os.curdir(), "Scene-Switch", "Config"):gsub("\\", "/")
    shader_dir = path.join(os.curdir(), "Scene-Switch", "Shaders"):gsub("\\", "/")
    model_dir = path.join(os.projectdir(), "Resources", "Models"):gsub("\\", "/")

    add_defines(
        "CONFIG_DIR=\"" .. config_dir .. "\"", 
        "SHADER_DIR=\"" .. shader_dir .. "\"", 
        "MODEL_DIR=\"" .. model_dir .. "\"")
    add_includedirs("../../src/")
    add_deps("ExampleBase")
    add_files("Scene-Switch/*.cpp")
    add_headerfiles("Scene-Switch/*.h")