target("OpenGLFrameworkIO")
    set_kind("static")
    add_deps("OpenGLFrameworkString")

    add_headerfiles("./*.h")
    remove_headerfiles("./*.test.h")
    add_files("./*.cpp")
    remove_files("./*.test.cpp")

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

    add_deps("OpenGLFrameworkIO")
    add_files(file)

end