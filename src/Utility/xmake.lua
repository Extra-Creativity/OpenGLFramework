target("OpenGLFrameworkUtility")
    set_kind("static")
    add_deps("OpenGLFrameworkIO", "OpenGLFrameworkString", "OpenGLFrameworkGenerator")

includes("IO", "String", "Generator", "GLHelper")