for _, file in ipairs(os.files("./*.cpp")) do

target(path.basename(file))
    set_kind("binary")
    add_includedirs("../src/")
    add_headerfiles("*.h")
    add_deps("OpenGLFramework")
    add_files(file)

end

includes("complex-examples")