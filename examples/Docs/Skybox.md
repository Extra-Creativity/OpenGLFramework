# Skybox

Skybox is a cube texture that acts as the background, which will hugely boost the reality of the scene. It's in fact a cube that doesn't change the view when camera translates, which makes an illusion that the scene is huge.

- [Skybox](#Skybox)
  * [Shaders](#shaders)
  * [SkyboxTexture](#SkyboxTexture)
  * [Vision illusion and optimization](#Vision-illusion-and-optimization)

Next doc can be found [here](NormalMap.md).

## Shaders

```glsl
// Skybox.frag
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}
```

`samplerCube` should be sampled by a 3D coordinate in $[-1,1]^3$, so we just get it from position of cube in $[0,1]^3$.

```glsl
// Skybox.vert
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = (aPos - vec3(0.5, 0.5, 0.5)) * 2; // aPos is in [0,1]
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

We also add a blend with reflection of skybox on the model:

```glsl
#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texCoords;

uniform sampler2D diffuseTexture1;
uniform samplerCube skybox;
uniform vec3 cameraPos;

out vec4 FragColor;

void main()
{
    vec3 incident = normalize(position - cameraPos);
    vec3 reflection = reflect(incident, normalize(normal));
    FragColor = vec4(texture(skybox, reflection).rgb, 1.0) * 0.5 +
				vec4(texture(diffuseTexture1, texCoords).rgb, 1.0) * 0.5;
    return;
}
```

## SkyboxTexture

A skybox needs 6 facets of cube, but all-in-one picture like this is usually provided too:

![](../../Resources/Models/Skybox/FS000_Day_01.png)

So we provide several ways to initialize skybox texture. You can:

+ `(path[, append={"_px", "_nx", "_py", "_ny", "_pz", "_nz"}, config=default])`: 6 split textures, with different suffix, e.g. `a_px.png`. Path should be complete path with no suffix, e.g. `/dir/a.png`.
+ `(paths[, config=default])`, designating all 6 paths manually.
+ `(path, segmentType[, config=default])`: use all-in-one texture; currently we only support `HorizontalLeft`, i.e. four facets at the horizontal and three two facets on the left side of vertical columns.

In our program, it's just like this:

```c++
auto skyBox = file.rootSection("option") == "split" ? 
    Core::SkyBoxTexture{ pathsSection("split_skybox_dir") } :
	Core::SkyBoxTexture{ pathsSection("skybox"), 
          Core::SkyBoxTexture::TextureSegmentType::HorizontalLeft };
```

You still need to bind skybox manually on the shader:

```c++
auto BindSkybox = 
[id = skyBox.GetID()](int textureBeginID, const Core::Shader& shader)
{
    Core::SkyBoxTexture::BindTextureOnShader(
        textureBeginID, "skybox", shader, id);
};
```

## Vision illusion and optimization

Since we need to cancel view change of skybox when translating camera, we need to reset the translation column in the view matrix:

```c++
auto initialViewMatTranslation = frontCamera.GetViewMatrix()[3];
auto viewMat = glm::mat4{ glm::mat3{ frontCamera.GetViewMatrix() } };
viewMat[3] = initialViewMatTranslation;
```

Also, we need to draw the skybox to the deepest position(i.e. `z=1`), so we need to change the vertex shader of the skybox like this:

```glsl
void main()
{
    TexCoords = (aPos - vec3(0.5, 0.5, 0.5)) * 2; // aPos is in [0,1]
    vec4 pos = projection * view * model * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // finally z = w/w = 1
}
```

Also, we need to set `glDepthFunc(GL_LEQUAL)` so that `depth=1` will not be culled; that's because clear depth is `1`, `GL_LESS` will discard all results that `depth=1`. Besides, we draw skybox finally since it's deeper than any other models, so this will boost performance because fragment shader can be called less for depth testing.

> Final word: if you resize the window, the scene will be distorted since we always use the initial width and height of window for projection. You may change it yourself if you want to make resize as expected.

After reading this doc, you'll fully understand what we do by reading the source code in `examples/SkyBox.cpp`.