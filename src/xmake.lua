target("OpenGLFramework")
    set_kind("static")
    add_deps("OpenGLFrameworkUtility", "OpenGLFrameworkCore")

for _, file in ipairs(os.files("./*.test.cpp")) do

target(path.basename(file))
    set_kind("binary")

    add_packages("catch2")
    -- to use Catch2WithMain.
    on_config(function(target)
        local _, _, toolset = target:tool("cxx")
        if toolset["name"] == "msvc" then
            target:add("ldflags", "/SUBSYSTEM:CONSOLE")
        end
    end)

    add_deps("OpenGLFramework")
    add_files(file)

end
includes("Utility", "FrameworkCore")
