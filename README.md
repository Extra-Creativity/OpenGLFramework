# README

In simple scenes, most of the code we write in OpenGL is drab and dreary and basically needs no or only slight changes. It's also quite a painful course to install dependencies. OpenGLFramework is a clean and easy-to-use wrapper for OpenGL.

## Annoucement

1. Currently, you can see `examples/complex-examples/SoftShadow-Advanced` to see PCSS implemented by our framework.
2. We're forging ahead for v1.3, where we'll dramatically refactor the code. There may be great breaking on APIs and docs to make it more convenient for users.

## Table of Contents

* [Build](#build)
* [Usage](#usage)
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

> Catch2 is needed only if you want to run unit tests.

After it prompts `build ok!`, you can enter `xmake run overall.test` to run the program and you get : 

![](result.png)

Model credits : miHoYo and [观海子](https://space.bilibili.com/17466365?spm_id_from=333.337.0.0)。See copyrights [here](#copyrights).

## Usage

We write docs for exmample programs in `exmaples/Docs`. See it for practical usage.

## Advantages

1. Faster loading speed: The most common code for OpenGL framework is in [learnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL), so we benchmark the total cost of creating Window, loading models, loading shaders and establishing the camera of ours and learnOpenGL's.

   |                      | Windows 10 | Ubuntu 20.04 |
   | -------------------- | ---------- | ------------ |
   | LearnOpenGL, release | 3.26205s   | 1.93490s     |
   | Ours, v1.0, release  | 0.72038s   | 0.645622s    |
   | Ours, v1.1, release  | 0.662553s  | 0.521885s    |

   Note that our CPU is Intel Core i7 and the model has 61434 vertices and 20478 facets. It indicates that we make it about four to five times faster than the baseline in the latest version.

2. Easier-to-use interface: We wrap the OpenGL code in RAII style, hiding trivial and boring inner details for the most common features. You can dive into writing proper shaders.

3. One-stop dependencies installation: It's widely known that OpenGL needs a bunch of dependencies which disturbs users a lot. Through XMake, we make it quite easy.

4. Support UTF-8 path : learnOpenGL may only supports ASCII path; we support UTF-8 path. In fact, the example model has textures that have Chinese characters.

5. Dynamic configuration: we support `.ini` configuration file format, so you can load them in run time and minimize the re-compilation when you want to change them.

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

+ `using enum`.

+ ...

Those methods are trivial compared to other parts, and will not or only very slightly affect performance, so you can substitute them relatively easily with C++17 code. But we recommend you to use C++20 to save that bother.

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
+ Nested namespace.

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

### v1.2 - 2023.5.21

+ More flexible framebuffer, e.g. you can attach more buffers and use them as textures.
+ Add PCSS example, with ImGUIHelper and AssetLoader.
+ Skybox texture, including six textures or all-in-one texture.
+ More powerful `MainWindow`, supporting more mouse events and screenshots.
+ Support screenshot of framebuffer.

## <span id="TODO">TODO</span>

+ `std::optional` is unnecessary if `std::reference_wrapper` is nullable(but in fact it isn't). We kind of regard it as an artifact of the standard library, and we may change them to pointers.
+ We may try to load meshes in parallel. This is not trivial for meshes will share textures so that `TexturePool` that uses STL is not thread-safe.
+ We may try to load data for every single mesh in parallel. This is not deterministic for huge models are likely to be divided into relatively medium-level meshes, so that the cost of each mesh is not worthwhile to create threads. However, we still reserve such possibilities because we use many `std::for_each` so that `std::execution` may benefit the range-based loop when [P2408R5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2408r5.html) for C++23 is implemented by all mainstream compilers.
+ `std::ranges::to<>` for convenient range conversion in C++23.
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
