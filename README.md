# README
---
This project is upgrading to version 1.1, which fixs a bunch of bugs, makes the code style uniform, adds dynamic configuration and so on. The README will be updated once the code is refactored completely, so the v1.0 doc below is temporarily obselete.
---

Most of the code we write in OpenGL is drab and dreary and basically needs no or only slight changes. It's also quite a painful course to install dependencies. OpenGLFramework is a clean and easy-to-use wrapper for OpenGL.

## Table of Contents

* [Build](#build)
* [Usage](#usage)
  + [MainWindow](#mainwindow)
  + [Framebuffer](#framebuffer)
  + [Vertex](#vertex)
  + [Transform](#transform)
  + [Texture](#texture)
  + [Mesh](#mesh)
  + [Shader](#shader)
  + [Camera](#camera)
  + [IOExtension](#ioextension)
  + [GPUExtension](#gpuextension)
* [Advantages](#advantages)
* [Build Tool](#build-tool)
* [Customized Builder](#builder)
* [Dependencies](#dependencies)
* [Compiler requirements](#compiler-requirements)
  + [C++20](#c++20)
  + [C++17](#c++17)
  + [cuda](#cuda)
* [Copyrights](#copyrights)

## Build

> All platforms need git commands.

Linux : 

>  Here we require to install g++-11; if you have the later version of gcc/g++, skip it and see [Customized Builder](#builder).

```bash
sudo apt update && sudo apt install gcc-11 g++-11
sudo apt install libxi-dev
(wget https://xmake.io/shget.text -O -)
cd framework/dir
xmake
```

> Maybe `export MESA_GL_VERSION_OVERRIDE=3.3`  is needed in`~/.bashrc` to use opengl 3.3.

Windows:

Download xmake at https://github.com/xmake-io/xmake/releases, and run `...-install.exe`, then :

```bash
cd framework/dir
xmake
```

MacOS : 

```bash
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install xmake
cd framework/dir
xmake
```

If XMake fails to install any of the packages, try the failed part of code below : 

```bash
xrepo install glfw
xrepo install glm
xrepo install glad
xrepo install imgui
xrepo install stb
xrepo install assimp
```

You can also refer to [this website](https://xmake.io/#/guide/installation) if you use other platforms or your installation fails with methods above.

After it prompts `build ok!`, you can enter `xmake run` to run the program and you get : 

![](result.png)

Model credits : miHoYo and [观海子](https://space.bilibili.com/17466365?spm_id_from=333.337.0.0)。See copyrights [here](#copyrights).

## Usage

> NOTE : You can customize any part by rewriting inner OpenGL code as you need. Besides, we strongly recommend you to read the code in `main.cpp` rather than read the usage directly, because we think the code is more intuitive.

You always need to call `InitContext()` first before using components of this project, and `EndContext()` if you don't use them any more.

### MainWindow

+ Initialization : `size_t width, size_t height, const char* title`.

+ `Register(func)` : It's recommended that you should use a lambda expression as the parameter; any out-of-scope local variables that need to be used in the scope should be captured by the `[]`. The registered functions will be executed sequentially as if looped in main.

  We wrap the GLFW so that it can accept variables through captures rather than only static or global variables.

+ `MainLoop(vec4 color)` : Begin the loop until closing the window. Note that `color` will be set before any execution of registered functions.

+ `Close()`: close the window.

+ `BindScrollCallback/ BindCursorPosCallback(func)`: when the mouse scrolls/ moves, the binded function will be called automatically.

  > Note that `BindCursorPosCallback` will make ImGui cannot detect the mouse event. See [here](https://stackoverflow.com/a/72509936/15582103) for more information.

+ `BindKeyPressing/BindKeyPressed/BindKeyReleasing/BindKeyReleased<keycode> (func)`: when the key is pressing/ pressed once/ releasing/ released once, the binded function will be called automatically.

Note that the logic of `MainWindow` assumes that there is only one instance (and we express it by a singleton-detected bool) because ImGui only supports binding a single GLFW window in its context. Besides, you can customize any needed functions in `MainWindow.h/.cpp` by imitating the code there.

Also, `MainWindow` uses a lot of `unordered_map`; if the binded functions will not be changed, you can use `std::vector` instead to get slight performance improvement.

### Framebuffer

Framebuffer is used to render an off-screen scene.

+ Initialization : `size_t width, size_t height bool needDepthTesting`, but optional.
+ `resize(size_t width, size_t height)`, resize the inner buffer.
+ If you want to show the scene in an ImGui Window, call `ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(frameBuffer.textureColorBuffer)), ImGuiWindowSize, { 0, 1 }, { 1, 0 });` in a ImGui context.

### Vertex

It has some variables that represent the vertex, like the position, normal coordinates and texture coordinates. Currently we assume there is only one set of the texture coordinates.

> In the future this may be adjusted for more features. What we can think is to use reflection so that we can automatically traverse the member and assign them through `glEnableVertexAttribArray`, thus making the customized `Vertex` struct possible. However, zero-cost reflection is supported only by C++23, which has quite a long way to go to use, so maybe I will consider other ways.

### Transform

Unity-like, with `vec3 position`, `quaternion rotation`, `vec3 scale` and methods :

+ `Rotate` by euler angles/ quaternions/ axis-angle 
+ `Translate(vec3)`: just move position.
+ `GetModelMatrix()`: get the model matrix caused by this transformation.

### Texture

+ Initialize : `std::filesystem::path texturePath`.

You can get its OpenGL ID through the member ` textureID`.

### Mesh

It has some variables that represent the mesh, like vertices, triangles, textures. It may not be exposed to the users, because it's mainly used for assimp adjustment. However, you may call `Draw(shader)/ Draw(shader, framebuffer)` to show a part of the model.

### Model

With a `Transform` variable to represent its state; sub-meshes that compose the model and all their textures.

+ Initialization : `std::filesystem::path modelPath, bool textureNeedFlip`; any format of path will be accepted.
+ `Draw(shader) / Draw(shader, framebuffer)`: use the shader to draw the model; if you want to render it on a framebuffer, pass it as the second parameter.

### Shader

+ Initialization : `std::filesystem::path vertexShaderPath, std::filesystem::path fragmentShaderPath` or `std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath`.
+ `Activate`: before you actually use the shader, you need to activate it.
+ `Set...` : set uniform variables in the shader.

Besides, you need always write your actual shader files like this :

```glsl
// version should be at least 330.
#version 330 core
    
// .vert layout should always be coded as below temporarily.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

// if you want use texture, name it with diffuseTexture/specularTexture + number(started from 1).
uniform sampler2D diffuseTexture1;
uniform sampler2D specularTexture1;
// ...
```

### Camera

It has some variables indicating its state, like `movementSpeed`, `mouseSensitivity`, `rotationSpeed`, `fov`. They are not actually necessary to a camera and you can delete them if you want.

+ Initilization : `vec3 position, vec3 up, vec3 front`; we don't require the `dot(up, front) = 0`, but just `cross(up, front) != {0,0,0}`. We will orthonormalize them in the process of initialization.

+ `Front()/Back()/Up()/Down()/Left()/Right()`
+ `GetViewMatrix()` : Get the view matrix determined by the camera paramters.
+ `Rotate/Translate` : similar to `Transform` `Rotate/Translate`, providing three methods to rotate the camera/ move the position.

### IOExtension

+ `ReadAll(std::filesystem::path path)` : return all contents of a file in the path.
+ `LogError(std::source_location location, std::string_view errorInfo)`: display the error information in the location; The first parameter is recommended to be set as `std::source_location::current()`.

### GPUExtension

+ `template<typename T> GPUvector(T* cpuPtr, size_t size)/ (size_t size)` : copy data at cpuPtr to GPU/ allocating `size * sizeof(T)` memory in GPU.
+ `GPUSynchronize()` : synchronize all kernels in GPU.
+ `size()` : get size.
+ `dataSymbol()` : get GPU ptr; actually it's not a valid address in CPU, so we call it a symbol.
+ `ToCPUVector(T* cpuPtr)` : copy data back to cpuPtr.
+ `struct GPUconfig` : set the configuration of the kernel, or in other words, a/ b in `<<<a, b>>>`. `m_needSynchronize` is to indicate whether the execution kernel should be synchronized.
+ `LoadVertices` : a test version of loading mesh through GPU.

## Advantages

1. Faster loading speed : The most common code for OpenGL framework is in [learnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL), so we benchmark the total cost of creating Window, loading models, loading shaders and establishing the camera of ours and learnOpenGL's.

   |             | Windows 10 | Ubuntu 20.04 |
   | ----------- | ---------- | ------------ |
   | LearnOpenGL | 3.26205s   | 1.93490s     |
   | Ours        | 1.09038s   | 0.645622s    |

   Note that our CPU is Intel Core i7 and the model has 61434 vertices and 20478 facets. It indicates that we make it about three times faster than the baseline.

2. Easier-to-use interface : We wrap the OpenGL code in RAII style, hiding trivial and boring inner details for most common features. You can dive into wring proper shaders.

3. One-stop dependencies installation : It's widely known that OpenGL needs a bunch of dependencies which disturb users a lot. Through XMake, we make it quite easy.

4. Support UTF-8 path : learnOpenGL only supports Ascii path; we support UTF-8 path. In fact, the example model has textures that have Chinese characters.

## Build Tool

We use XMake as our build tool. Because : 

+ For CMake, (we think) it's miserable to use convenient functions like `find_package` in Windows and usually third-party libraries need to be included in folders. 
+ For vcpkg & plug-ins, it's a really good choice in Windows but a little bit unsatisfying for cross-platform code.

XMake is a lua-based convenient tool for cross-platform code written by a Chinese developer, [@waruqi](https://github.com/waruqi). It's equipped with a package manager, XRepo, which has most common libraries. After installing them, you can use `find_package`-like functions in `xmake.lua`. It combines the merits of CMake and vcpkg while having at least same building speed compared to other mainstream building tools.

> In fact Lua knowledge is nearly unnecessary for basic build tasks.
>
> We believe that XMake is easier to code than CMake, but it's a pity that documents of XMake are still developing and not as satisfying as we expect. We are still struggling to study now.
>
> XMake can also generate IDE project files, see [here](https://xmake.io/#/plugin/builtin_plugins?id=generate-ide-project-files).

You can check [XMake Github website](https://github.com/xmake-io/xmake) for more infomation.

## <span id="builder">Customized Builder</span>

You can also use your existing vcpkg libraries or enable cuda.

```lua
set_project("OpenGLFramework")
set_version("1.0.0")
set_xmakever("2.6.1")

set_languages("cxx20")
-- you may change or delete it as you want.
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
```

## Dependencies

`opengl3.3`, `glfw`, `glad`, `glm`, `assimp`, `stb`, `imgui`, `imgui-[glfw]` and `imgui-[opengl3]`.

If you use xmake, they will be installed automatically. Note that `assimp` installation may need some time because it's a little bit large.

## Compiler requirements

By default, we use `g++-11` as the compiler for Linux and default toolchains for other platforms (e.g. msvc for Windows).

### C++20

Considering that some core features (like `module`) in C++20 are not easy for those who haven't studied them to convert back to C++17 code (like header-style), we only use minor new properties in C++20. If you cannot use C++20 features, you can detect and replace them easily.

What we use in C++20：

+ `<numbers>`, for `std::numbers::pi_v<f>` to get π.

+ `<version>`, to check whether `<format>` is supported. If it is, `<format>` will be used.

  > Sadly gnu doesn't support `<format>` yet (even in the latest version), which we think is a really significant feature in C++20.

+ `[[likely]]` and `[[unlikely]]` attributes, to clearly influence branch-predict policies.

+ `<source_location>`, to log error info.

Those methods are trivial compared to other parts, and will not or only very slightly affect performance, so you can substitute them easily with C++17 code.

### C++17

There are also some C++17 features that may need extra libraries to support in old versions of some compilers, so we also list them as follows:

What we use in C++17：

+ structured binding, for getting the pair/tuple return value.

+ Class template argument deduction (CTAD), to reduce code of some unnecessary types.

+ `std::unordered_map::insert_or_assign()`.

+ **`<execution>`, to load models in parallel in multi-core machines.**

+ **`<filesystem>`.**

+ `[[maybe_unused]]` attributes, to eliminate not-used warnings for ImGui needs `auto& io = ImGui::GetIO()` to monitor some events even though `io` is not used in the user's code block.

+ `<string_view>`, to replace some of the `const char*` in modern C++.

The bold parts represent that code with those features is crucial, and it's not easy to re-code. Thus, we recommend you to use a compiler that at least supports C++17.

### cuda

CUDA is only optional in our code, and it's disabled by default. There are only some convenient wrappers in `GPUExtension.h/.cu`, and `Model.cpp` use them to load models. For slow memory transfer between CPU and GPU, enabling cuda performs even worse in our own platform. However, we still reserve that in case your GPU is dramatically fast. You can uncomment `chrono` code at the beginning of `main` to get a small benchmark.

> You may also need adding contents in ~/.bashrc in Linux if GUI crashes sometimes :  
>
> ```bash
> export PATH=/usr/local/cuda-version/bin:$PATH
> export LD_LIBRARY_PATH=/usr/local/cuda-version/lib64:$LD_LIBRARY_PATH
> ```
>
> where `version` is you cuda version like `11.1`.

## <span id="copyrights">Copyrights</span>

This project owns a MIT license in the public domain.

For the model : 

> This is the translation of original model README.
>
> Thanks for downloading this model!
>
> You can : 
>
> + improve the physical effects, correct possible bugs on model weights and facial expressions;
> + Change the color and outfit properly, add spa, toon and so on.
>
> You should NOT : 
>
> + distribute again, dispatch parts to use in other models; 
> + use for 18+ works, extreme religious propagandas, sanguinary grisly strange works, assault and battery, and so on.
> + use for commercial purposes.
>
> The responsibility of all possible negative outcomes resulted from others' use of this model will not be taken by HoYoverse and the model releaser, but taken by the user.
>
> model provider : HoYoverse
>
> model releaser : 观海
>
> Final interpretation power belongs to HoYoverse.
>
> PLEASE obey rules above.

