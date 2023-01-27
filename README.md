# README

In simple scenes, most of the code we write in OpenGL is drab and dreary and basically needs no or only slight changes. It's also quite a painful course to install dependencies. OpenGLFramework is a clean and easy-to-use wrapper for OpenGL.

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
  + [IniFile](#IniFile)
  + [StringExtension](#StringExtension)
* [Advantages](#advantages)
* [Build Tool](#build-tool)
* [Customized Builder](#builder)
* [Dependencies](#dependencies)
* [Compiler requirements](#compiler-requirements)
  + [C++20](#cpp20)
  + [C++17](#cpp17)
* [Update Information](#Update-Information)
* [TODO](#TODO)
* [Copyrights](#copyrights)

## Build

> All platforms need git commands.

First, you need to install [xmake](https://github.com/xmake-io/xmake)(You can visit the website when you want to get up-to-date installation methods or use other platforms):

+ Linux :

    ```bash
    sudo apt update && sudo apt install gcc-11 g++-11
    sudo apt install libxi-dev
    (wget https://xmake.io/shget.text -O -)
    ```
    
    > Maybe `export MESA_GL_VERSION_OVERRIDE=3.3`  is needed in`~/.bashrc` to use OpenGL 3.3.


+ Windows: Download xmake at https://github.com/xmake-io/xmake/releases, and run `...-install.exe`.

+ MacOS : 

    ```bash
    ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
    brew install xmake
    ```

Then build the `overall.test` target:

```bash
cd framework/dir
xmake overall.test
```

If XMake fails to install any of the packages, try code below for the failed parts: 

```bash
xrepo install glfw
xrepo install glm
xrepo install glad
xrepo install imgui
xrepo install stb
xrepo install assimp
```

> Catch2 is optional if you want to run unit tests.

After it prompts `build ok!`, you can enter `xmake run overall.test` to run the program and you get : 

![](result.png)

Model credits : miHoYo and [观海子](https://space.bilibili.com/17466365?spm_id_from=333.337.0.0)。See copyrights [here](#copyrights).

## Usage

> NOTE : You can customize any part by rewriting the inner OpenGL code as you need. Besides, we strongly recommend you to read the code in `main.cpp` rather than read the usage directly because we think the code is more intuitive.

You always need to call `[[maybe_unused]] auto& contextManager = ContextManager::GetInstance()` first before using the components of this project. It will initialize context when the first time this function is called and end context when the whole program ends.

----

**Components below are in namespace `OpenGLFramework::Core`, and headers are in `FrameworkCore/`.** They will not try to catch exceptions, and the thrown exceptions are all caused by the standard library. When OpenGL errors occur, it will continue to run while logging the error information rather than throw exceptions.

### MainWindow

+ Initialization : `size_t width, size_t height, const char* title`.

+ `Register(func)` : It's recommended that you should use a lambda expression as the parameter; any out-of-scope local variables that need to be used in the scope should be captured by the `[]`. The registered functions will be executed sequentially as if looped in main.

  We wrap the GLFW so that it can accept variables through captures rather than only static or global variables.

+ `MainLoop(vec4 color)` : Begin the loop until closing the window. Note that `color` will be set before any execution of registered functions.

+ `Close()`: close the window.

+ `BindScrollCallback/ BindCursorPosCallback(func)`: when the mouse scrolls/ moves, the bound function will be called automatically.

  > Note that `BindCursorPosCallback` will make ImGui cannot detect the mouse event. See [here](https://stackoverflow.com/a/72509936/15582103) for more information.

+ `BindKeyPressing/BindKeyPressed/BindKeyReleasing/BindKeyReleased<keycode> (func)`: when the key is pressing/ pressed once/ releasing/ released once, the bound function will be called automatically.

Note that the logic of `MainWindow` assumes that there is only one instance (and we express it by a thread-unsafe singleton-detected bool) because ImGui only supports binding a single GLFW window in its context. Besides, you can customize any needed functions in `MainWindow.h/.cpp` by imitating the code there.

Also, `MainWindow` uses a lot of `std::unordered_map`; if the bound functions will not be changed, you can use `std::vector` instead to get a slight performance improvement.

### Framebuffer

Framebuffer is used to render an off-screen scene.

+ Initialization: `size_t width, size_t height bool needDepthTesting`, but optional. The default parameters are `(1000, 1000, true)`.
+ `Resize(size_t width, size_t height)`, resize the inner buffer.
+ If you want to show the scene in an ImGui Window, call `ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(frameBuffer.textureColorBuffer)), ImGuiWindowSize, { 0, 1 }, { 1, 0 });` in a ImGui context.

You can also adjust the member variable `backgroundColor` to use different color for the framebuffer.

### Transform

Unity-like, with `vec3 position`, `quaternion rotation`, `vec3 scale` and methods :

+ `Rotate` by Euler angles/ quaternions/ axis-angle 
+ `Translate(vec3)`: just move the position.
+ `GetModelMatrix()`: get the model matrix caused by this transformation.

### Texture

+ Initialize : `std::filesystem::path texturePath`.

You can get its OpenGL ID through the member ` ID`.

### Mesh

#### BasicTriMesh

Only stores vertices and triangles. It's pure model without rendering resources.

#### BasicTriRenderMesh

Derived from `BasicTriMesh`, owning `vertesAttributes` for texture coordinates and normals and rendering resources. It may not be exposed to the users, because it's mainly used for assimp adjustment. However, you may call `Draw(shader)/ Draw(shader, framebuffer)` to show a part of the model.

### Model

#### BasicTriModel

Just an array of `BasicTriMesh` and its transformation. It should be initialized by the path of the model.

#### BasicTriRenderModel

Array of `BasicTriRenderMesh` and all their textures. `Transform transform` is also provided.

+ Initialization : `std::filesystem::path modelPath, bool textureNeedFlip`; any format of path will be accepted.
+ `Draw(shader) / Draw(shader, framebuffer)`: use the shader to draw the model; if you want to render it on a framebuffer, pass it as the second parameter.

### Shader

+ Initialization : `std::filesystem::path vertexShaderPath, std::filesystem::path fragmentShaderPath` or `std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath`.
+ `Activate`: before you actually use the shader, you need to activate it.
+ `Set...`: set uniform variables in the shader.

Besides, you always need to write your actual shader files like this :

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
+ `GetPosition()`.
+ `Front()/Back()/Up()/Down()/Left()/Right()`
+ `GetViewMatrix()`: Get the view matrix determined by the camera parameters.
+ `Rotate/Translate`: similar to `Transform` `Rotate/Translate`, providing three methods to rotate the camera/ move the position.
+ `RotateAroundCenter(float angle, vec3 axis, vec3& center)`.

----

**Components below are in namespace `OpenGLFramework::IOExtension`, and headers are in `Utility/IO`.**

### IOExtension

+ `ReadAll(std::filesystem::path path)`: return all contents of a file in the path.
+ `LogError(std::source_location location, std::string_view errorInfo)`: display the error information in the location; The first parameter is recommended to be set as `std::source_location::current()`.
+ `LogStreamStatus`: log the status of the file stream and throw `std::runtime_error` if it's bad.

### <span id="IniFile">IniFile</span>

You can use `.ini` file for dynamic configuration so the burden of re-compiling will be released. For `.ini` format, please refer to [wiki](https://en.wikipedia.org/wiki/INI_file). For the optional features of `.ini`, we support `#` as comments and we also support sub-sections. Notice that `;` or `#` must be the first non-blank character to denote the line as a comment line.

`IniFile` is initialized by its path. The only member variable you can use is `Section rootSection`. 

`Section` is just an implementation of section in `.ini` file. We provide two kinds of data-getter:

+ `operator[]/()`: The former is for the section index, and the latter is for the entry index. These two APIs don't check the existence of the key and don't normalize(i.e. trim and make string case-insensitive required by `.ini`). Subsections are not supported. For example, if you want to index entry `C` in  `A.B`, you need to use `["a"]["b"]("c")`.
+ `GetSubsection/GetEntry`: These two APIs will normalize the key, divide the subsection indices and check existence of the key. The return value is `std::optional<>` so that you need to check `std::nullopt`. We may change it to pointers in the future so that you need to check `nullptr`. For example, if you want to index entry `C` in  `A.B`, you need to use (here we don't check null) `GetSubsection("A.B")->get().GetEntry("C")`.

Other utility functions like `std::string IniFileNameNormalize(std::string_view)` in the global scope and `GetSubsectionSize/GetEntrySize` in the `Section` scope are also provided.

Besides, we use template so that you can use `std::map` to replace the default `std::unordered_map`. Any container that meets the required APIs(i.e.`find/operator[]/iterator`) of `std::unordered_map` can also be used.

----

**Components below are in namespace `OpenGLFramework::StringExtension`, and headers are in `Utility/String`**.

### <span id="StringExtension">StringExtension</span>

Only ASCII and UTF-8 are supported.

+ `CharAsciiToLower/StringAsciiToLower`: transform all English alphabets to lowercase.
+ `TrimBegin/TriEnd/Trim`: trim the blank characters in the string.

## Advantages

1. Faster loading speed: The most common code for OpenGL framework is in [learnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL), so we benchmark the total cost of creating Window, loading models, loading shaders and establishing the camera of ours and learnOpenGL's.

   |             | Windows 10 | Ubuntu 20.04 |
   | ----------- | ---------- | ------------ |
   | LearnOpenGL | 3.26205s   | 1.93490s     |
   | Ours, v1.0  | 1.09038s   | 0.645622s    |
   | Ours, v1.1  |            | 0.521885s    |

   Note that our CPU is Intel Core i7 and the model has 61434 vertices and 20478 facets. It indicates that we make it about four times faster than the baseline in the latest version.

2. Easier-to-use interface: We wrap the OpenGL code in RAII style, hiding trivial and boring inner details for the most common features. You can dive into writing proper shaders.

3. One-stop dependencies installation: It's widely known that OpenGL needs a bunch of dependencies which disturbs users a lot. Through XMake, we make it quite easy.

4. Support UTF-8 path : learnOpenGL may only supports ASCII path; we support UTF-8 path. In fact, the example model has textures that have Chinese characters.

## Build Tool

We use XMake as our build tool. Because : 

+ For CMake, (we think) it's miserable to use convenient functions like `find_package` in Windows and usually third-party libraries need to be included in folders. 
+ For vcpkg & plug-ins, it's a really good choice in Windows but a little bit unsatisfying for cross-platform code.

XMake is a Lua-based convenient tool for cross-platform code written by a Chinese developer, [@waruqi](https://github.com/waruqi). It's equipped with a package manager, XRepo, which has most common libraries. After installing them, you can use `find_package`-like functions in `xmake.lua`. It combines the merits of CMake and vcpkg while having at least same building speed compared to other mainstream building tools.

> In fact Lua knowledge is nearly unnecessary for basic build tasks.
>
> We believe that XMake is easier to code than CMake, but it's a pity that documents of XMake are still developing and not as satisfying as we expect. We are still struggling to study now.
>
> XMake can also generate IDE project files, see [here](https://xmake.io/#/plugin/builtin_plugins?id=generate-ide-project-files).

You can check [XMake Github website](https://github.com/xmake-io/xmake) for more information.

## Dependencies

`opengl3.3`, `glfw`, `glad`, `glm`, `assimp`, `stb`, `imgui`, `imgui-[glfw]` and `imgui-[opengl3]`. `catch2` is optional if you need unit tests.

If you use xmake, they will be installed automatically. Notice that `assimp` installation may need some time because it's a little bit large.

## Compiler requirements

### <span id="cpp20">C++20</span>

Considering that some core features (like `module`) in C++20 are not easy for those who haven't studied them to convert back to C++17 code (like header-style), we only use minor new properties in C++20. If you cannot use C++20 features, you can detect and replace them easily.

What we use in C++20：

+ `<numbers>`, for `std::numbers::pi_v<f>` to get π.

+ `<version>`, to check whether `<format>` is supported. If it is, `<format>` will be used.

  > `<format>` is not supported until gcc13, which is not released currently.

+ `[[likely]]` and `[[unlikely]]` attributes, to clearly influence branch-predict policies.

+ `<source_location>`, to log error info.

+ `char8_t` for UTF-8 string. Use `char` in C++17.

+ `ranges`, for splitting string in `IniFile` and possible future parallelism(see TODO).

+ Concept, but only `StringExtension` uses very simple concept. You can relatively easy to remove them as you want.

Those methods are trivial compared to other parts, and will not or only very slightly affect performance, so you can substitute them easily with C++17 code.

### <span id="cpp17">C++17</span>

There are also some C++17 features that may need extra libraries to support in old versions of some compilers, so we also list them as follows:

What we use in C++17：

+ structured binding, for getting the pair/tuple return value.
+ Class template argument deduction (CTAD), to reduce code of some unnecessary types.
+ `std::unordered_map::insert_or_assign()`, **`std::unordered_map::try_emplace()`**.
+ ~~**`<execution>`, to load models in parallel in multi-core machines.**~~ **Temporarily not used, see TODO**.
+ **`<filesystem>`.**
+ `[[maybe_unused]]` attributes, to eliminate not-used warnings for ImGui needs `auto& io = ImGui::GetIO()` to monitor some events even though `io` is not used in the user's code block.
+ `[[fall_through]]` to indicate deliberate no ending `break` in switch-cases.
+ **`<string_view>`, to replace some of the `const char*` in modern C++.**
+ `<optional>` and `std::reference_wrapper`.

The bold parts represent that code with those features is crucial, and it's not easy to re-code. Thus, we recommend you to use a compiler that at least supports C++17.

## <span id="Update-Information">Update Information</span>

### v1.1 - 2023.1.26

+ Add `OpenGLFramework` namespace.
+ Completely optimize file organization.
+ Unify code style.
+ Optimize model loading to fit more for indexed-based geometry.
+ Split rendering and model to some extent.
+ Complete RAII-style resource management in all classes.
+ Delete `GPUExtension` for cross-platform ability. Thus, CUDA requirements are removed.
+ Add `StringExtension` and enrich `IOExtension`.
+ Add `IniFile` for dynamic configuration.
+ Add rather complete unit test with [Catch2](https://github.com/catchorg/Catch2).
+ Fix significant bugs. For example, use singleton `ContextManager` to replace manual `Init/EndContext`, so that window will be destroyed before context ending(previously it's not, which is logically wrong).

## <span id="TODO">TODO</span>

+ `std::optional` is unnecessary if `std::reference_wrapper` is nullable(but in fact it isn't). We kind of regard it as an artifact of the standard library, and we may change them to pointers.
+ We may try to load meshes in parallel. This is not trivial for meshes will share textures so that `TexturePool` that uses STL is not thread-safe.
+ We may try to load data for every single mesh in parallel. This is not deterministic for huge models are likely to be divided into relatively medium-level meshes, so that the cost of each mesh is not worthwhile to create threads. However, we still reserve such possibilities because we use many `std::for_each` so that `std::execution` may benefit the range-based loop when [P2408R5](#https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2408r5.html) for C++23 is implemented by all mainstream compilers.
+ `z` suffix for `size_t` and `std::ranges::to<>` for convenient range conversion in C++23.
+ More features like sky box and more complex example shaders.

## <span id="copyrights">Copyrights</span>

This project owns an MIT license in the public domain.

For the model : 

> This is the translation of the original model README.
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
