# Blinn Phong

As the first doc in all examples, here we'll explain code in `examples/BlinnPhong.cpp` in detail so that you can grasp provided APIs quickly.

- [Blinn Phong](#blinn-phong)
  * [Shaders](#shaders)
  * [Configuration](#configuration)
  * [Transform](#transform)
  * [Camera](#camera)
  * [ContextManager](#contextmanager)
  * [MainWindow](#mainwindow)
  * [Shaders](#shaders-1)
  * [Mesh and Model](#mesh-and-model)
  * [Appendix](#appendix)
    + [Basic bindings](#basic-bindings)
    + [Additional methods of MainWindow](#additional-methods-of-mainwindow)

Next doc can be found [here](ShadowMap.md).

## Shaders

Shader program is usually the core of rendering algorithms, and can be separately understood without C++ code of framework. First, let's see the vertex shader:

```glsl
// Shaders/BlinnPhong.vert
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    return;
}
```

Obviously, this vertex shader apply MVP transformation on the position, and transmit new position to the next shader. Notice that `gl_Position` and `FragPos` are different; the former is used to denote the position **viewed from camera**, the latter is the physical position in the world(it's needed since Blinn Phong needs distance between the fragment and the light).

> **Framework Related**: *We always provide position at location 0, normal at location 1 and texture coordinate at location 2. But you can omit some of them if it's not needed*.

Now let's see fragment shader:

```glsl
// Shaders/BlinnPhong.frag
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D diffuseTexture1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;

void main()
{
    vec4 baseColor = texture(diffuseTexture1, TexCoords);

    float ks = 100, kd = 50, ka = 0.1;
    int p = 32;

    vec3 shadePointToLightVec = lightPos - FragPos;
    float attenuateDistance = length(shadePointToLightVec);
    vec4 backgroundColor = vec4(1.0, 1.0, 1.0, 1.0);

    vec3 inVec = normalize(cameraPos - FragPos);
    vec3 outVec = normalize(shadePointToLightVec);

    vec3 halfVec = normalize(inVec + outVec);
    float specularCoeff = ks * pow(max(0, dot(halfVec, Normal)), p);
    float diffuseCoeff = kd * max(0, dot(inVec, outVec));

    FragColor = ((specularCoeff + diffuseCoeff) / 
                (attenuateDistance * attenuateDistance)) * baseColor * 
                vec4(lightColor, 1.0) + backgroundColor * ka;
    return;
}
```

The formula of Blinn-Phong shading is $L=L_s+L_d+L_a$, where specular lighting $L_s=k_s\frac{I}{r^2}\max{(0, h\cdot n)^p}$, diffuse lighting $L_d=k_d\frac{I}{r^2}\max{(0, \text{incident}\cdot \text{out})}$ and ambient shading $L_a=k_aI_a$. Just like this:

![](advanced_lighting_halfway_vector.png)

Incident is just the vector from the fragment to the camera, out is just the vector from the fragment to the light(or vice versa, equivalent here), and half vector is their sum with normalization.

> **Framework Related**: *We always provide speculative textures as* `specularTextureN` *and diffuse textures as* `diffuseTextureN`. *It's you who need to know what each texture is. Here it's just the only texture of model.*

## Configuration

Since models and shader files should be loaded from file, we need to provide their paths. Compared with hard-coded them in the source code, it's better to place them in a configuration file. OpenGLFramework provides an `ini` file parser in `Utility/IO/IniFile.h`. As one of basic configuration files, it's easier for users to understand and write, but less powerful than json. The format (we supported) is like:

```ini
# This is comment, which can only appear at the beginning.
; Another comment 这也是一条注释.
# You can add any blank characters at the beginning of end.
rootSectionKey = value0
[sectionName]
key = value1
[sectionName.subsectionName]
key2 = value2
# Particularly, ini file is case-insensitive for keys and section names
# For example, KEYNAME and keyName is seen as the same key/section.
# Thus, it's recommended to use underscore for name, e.g. key_name.
```

We support ASCII and UTF-8 characters currently. So to use the configuration file, you can:

```c++
std::filesystem::path configPath = "...";
IOExtension::IniFile file{ configPath };
```

Then, you can use `.rootSection` to get key-value pair in the root section(e.g. `rootSectionKey` here) and subsections(e.g. `sectionName` here):

```c++
auto& modelsSection = file.rootSection.GetSubsection("models")->get();
```

Notice that `IniFile` provides `GetSubsection(std::string)` and `operator[](std::string)` to get subsections.

+ The former will normalize the key first(i.e. trim blank characters and make the key case-insensitive). The latter will use the provided key directly.
+ The former will parse the key, i.e. you can access the subsection by `GetSubsection("section.subsection")`; the latter will use it as key directly.
+ The former will return `std::optional<std::reference_wrapper<Section>>`, which will be `std::nullopt` when the subsection doesn't exist. The latter will always return `Section&`, which will creates an empty subsection if it doesn't exist.

All in all, `GetXX` is provided for safety and `operatorXX` is provided for performance. Similarly, you can access entries by `GetEntry` and `operator()`. For example:

```c++
auto& nameSection = file.rootSection.GetSubsection("name")->get();
std::string windowName = nameSection("window_name");
```

> Note1: You can also get the container directly by `GetRawSubsections/GetRawEntries`, or get their size by `GetSubsectionSize/GetEntrySize`. 
>
> Note2: You can in fact specify the associative container used to store entries or subsections. We use `std::unordered_map` as default, and you may use `std::map` or `std::flat_map` in C++23 if you want.
>
> Note3: If you can ensure the existence of names, you can access an entry by:
>
> ```c++
> file.rootSection["sectionName"]["subsectionName"]("key"); // or
> (*file.GetSubsection("sectionName.subsectionName"))("key");
> ```
>
> But it's usually recommended to use `auto&` to get the section temporarily, so that you can access the key directly without jumping many times.

## Transform

Unity-like, with `vec3 position`, `quaternion rotation`, `vec3 scale` and methods defined in `FrameworkCore/Transform.h`:

+ `Rotate` by Euler angles/ quaternions/ axis-angle 
+ `Translate(vec3)`: just move the position.
+ `GetModelMatrix()`: get the model matrix caused by this transformation.

You can also manipulate `position/scale/rotation` directly, as we've exposed them as public members.

## Camera

Camera is the basic components in Graphics, whether for user's view or other purposes. We provide easy interface defined in `FrameworkCore/Camera.h` for it:

+ Initilization : `vec3 position, vec3 up, vec3 front`; we don't require the `dot(up, front) = 0`, but just `cross(up, front) != {0,0,0}`. We will orthonormalize them in the process of initialization.
+ `GetPosition/GetGaze/GetUp()`.
+ `Front()/Back()/Up()/Down()/Left()/Right()`
+ `GetViewMatrix()`: Get the view matrix determined by the camera parameters.
+ `Rotate/Translate`: similar to `Transform` `Rotate/Translate`, providing three methods to rotate the camera/ move the position.
+ `RotateAroundCenter(float angle, vec3 axis, vec3 center)`.

There are also some data members, i.e. `movementSpeed`, `mouseSensitivity`, `rotationSpeed` and `fov`. Except for `fov`, other three are provided for coder to mainpulate some input.

In this piece of code, we initialize it by `Core::Camera frontCamera{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} };`  and use it to set view matrix.

## ContextManager

To really start the context for OpenGL, you need `Core::ContextManager` defined in `FrameworkCore/ContextManager.h`. This is provided by an singleton, and it's done when you first call `ContextManager::GetInstance()`. The context is destroyed once the whole program exits. Anyway, when you want to use any rendering-related components, you need to call:

```c++
[[maybe_unused]] auto& contextManager = ContextManager::GetInstance();
```

only once in `main`.

> Note: `[[maybe_unused]]` is used to eliminate warnings since we do initialization work silently in the function, but it seems to do nothing in the compiler's view.

## MainWindow

`MainWindow` is an singleton GLFW window defined in `Framework/MainWindow.h`. You may register events and bind keys/buttons on it, get its width and height, get the time interval(i.e. delta time) or total time.

> Notice: Though GLFW allows multi-threaded windows, OpenGL 3.x doesn't fit with it well, so we obligate single-threading for rendering. But if there is any background computation without changing the total OpenGL state, you can still utilize multi-threading.

+ Initialization : `size_t width, size_t height, const char* title, visible=true`. If you want to hide the window initially, you can set the last argument `false`.

+ `Hide(bool hide = true)`: hide/show the window. 

+ `Register(func)` : You can provide almost any callable, e.g. lambda expression with captures, bound `std::functions`, functors, normal functions, etc.. The registered functions will be executed sequentially as if looped in main.

+ `ClearRoutines()`: clear all registered functions.

+ `MainLoop(vec4 color)` : Begin the loop until closing the window. Note that `color` will be set before any execution of registered functions.

+ `GetWidthAndHeight`.

+ `Close()`: close the window.

+ `SaveImage(path, needFlip=true)`: save the current frame in the path; you need to specify suffix as `.bmp/jpg/png`, otherwise it'll be saved as `png`.

+ `GetDeltaTime/GetCurrTime()`.

These APIs are mostly used for users; we also introduce other methods like binding at the end. We've already provided basic bindings, so you may not bother to write it yourself.

In our program, we use it like:

```c++
Core::MainWindow mainWindow{ 800, 600, windowName.c_str() };
SetBasicKeyBindings(mainWindow, frontCamera);
SetBasicButtonBindings(mainWindow, frontCamera);
SetBasicTransformSubwindow(mainWindow, sucroseModel);
mainWindow.Register([...]() { /* ... */ });
mainWindow.MainLoop({0, 0, 0, 1}); // black background.
```

## Shaders

Defined in `FrameworkCore/Shader.h`, shaders are in fact called "program" in OpenGL. You can provide paths of vertex shader, (optional) geometry shader and fragment shader, and we'll link them together. You can call `Activate` to use the program, and `SetXX` to set uniform variables in the shader. In our program, we use it like:

```c++
Core::Shader sucroseShader{
    sucroseSection(vertexShaderDir),
    sucroseSection(fragmentShaderDir)
};

sucroseShader.Activate();
sucroseShader.SetVec3("lightPos", lightPosition);
sucroseShader.SetVec3("lightColor", lightColor);
sucroseShader.SetVec3("cameraPos", frontCamera.GetPosition());
```

> Note: `Shader` is not copiable; it's only movable.

## Mesh and Model

We provide renderable model/mesh and non-renderable ones in `FrameworkCore/Model.h/Mesh.h`. A model is just a vector of mesh, with sharing texture pools and `Transform transform`.

For each `BasicTriMesh`, we provide only vertices and triangles, with methods `GetTriangleVerts` to get three vertices of the triangle. For each `BasicTriRenderMesh`, we additionally provide `Draw(shader[, preprocess, postprocess])`. 

The most frequently used one is `BasicTriRenderModel`, initialized by a path of model and `textureNeedFlip` boolean. We also provide a `Draw` method for it. You can access its sub-mesh too.

In the program, we use it like this:

```c++
Core::BasicTriRenderModel sucroseModel{ sucroseSection(modelDir)};
sucroseModel.Draw(sucroseShader);
```

With all these hints, you can fully understand what the program does by reading the source code yourself!

> Final word: you can use `FrameworkCore/Core_All.h` to include all headers in framework core, which may drag compilation time but be more convenient.

## Appendix

### Basic bindings

`W/A/S/D`: go forwards/left/right/forwards in **world coordinate.**

`Up/Down/Left/Right`: rotate the camera up/down/left/right.

`Mouse RightButton`: rotate the camera around model by dragging the mouse.

### Additional methods of MainWindow

+ `GetCursorPos`: as its name.

+ `BindScrollCallback/ BindCursorPosCallback(func)`: when the mouse scrolls/ moves, the bound function will be called automatically.

  > Note that `BindCursorPosCallback` will make ImGui cannot detect the mouse event. See [here](https://stackoverflow.com/a/72509936/15582103) for more information.

+ `BindKeyPressing/BindKeyPressed/BindKeyReleasing/BindKeyReleased<keycode> (func)`: when the key is pressing/ pressed once/ releasing/ released once, the bound function will be called automatically.

+ `BindMouseButtonPressing/BindMouseButtonPressed/BindMouseButtonReleasing/BindMouseButtonReleased<keycode> (func)`: similar as above, but use mouse button.

+ `GetKeyState(key)`: return `GLFW_PRESS/GLFW_RELEASE` if key is pressed/released.

+ `SetInputMode(mode, value)`: same as `glfwSetInputMode(thisWindow, mode, value)`.

+ `GetNativeHandler()`: return `GLFWwindow*`, which may be used to cooperate with unsupported methods above.

You may see `BasicSettings.h` for their usage.