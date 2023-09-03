target("ExampleBase")
    set_kind("static")
    add_deps("OpenGLFramework")
    add_includedirs("../../src/")
    add_files("Base/*.cpp")
    add_headerfiles("Base/*.h")

for _, dirName in ipairs(os.dirs("./*")) do

if(dirName ~= "Base") then

target(dirName)
    set_kind("binary")
    config_dir = path.join(os.curdir(), dirName, "Config"):gsub("\\", "/")
    shader_dir = path.join(os.curdir(), dirName, "Shaders"):gsub("\\", "/")
    model_dir = path.join(os.projectdir(), "Resources", "Models"):gsub("\\", "/")

    add_defines(
        "CONFIG_DIR=\"" .. config_dir .. "\"", 
        "SHADER_DIR=\"" .. shader_dir .. "\"", 
        "MODEL_DIR=\"" .. model_dir .. "\"")
    add_includedirs("../../src/")
    add_deps("ExampleBase")
    add_files(dirName .. "/*.cpp")
    add_headerfiles(dirName .. "/*.h")

end
end