set_project("OpenGLFramework")
set_version("1.1")
set_xmakever("2.6.1")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")
set_languages("cxx20")
add_cxxflags("-Wall")

on_load(function (target)
    local projectDir = os.projectdir()
    local scriptDir = target:scriptdir()
    local relativeDir = path.relative(scriptDir, projectDir)
    target:set("targetdir", path.join("$(buildir)", "$(plat)", "$(arch)", "$(mode)", relativeDir))

    -- the root dir, don't set group
    if(relativeDir == ".") then 
        return
    end

    local relativeDirWithoutBackslash, _ = string.gsub(relativeDir, "\\", "/")
    target:set("group", relativeDirWithoutBackslash)
end)

-- disable other sources.
add_requires("catch2", { system = false, optional = true })
-- add opengl-related library.
add_requires("glfw", "glad", "glm", "assimp", "stb")
add_requires("imgui", {configs={glfw_opengl3 = true}})

-- add global libraries
add_packages("catch2", "glfw", "glad", "glm", "assimp", "imgui", "stb")

includes("src")
includes("examples")
