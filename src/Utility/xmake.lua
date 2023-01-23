target("OpenGLFrameworkUtility")
    set_kind("static")
    add_deps("OpenGLFrameworkIO", "OpenGLFrameworkString")

includes("IO", "String")