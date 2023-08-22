# Shadow Map

The core of shadow in graphics is shadow map, i.e. prepare a virtual camera at the position of light, whose depth buffer will be the shadow map. If the distance of fragment is greater than the corresponding depth in the shadow map, then it's blocked, which produces shadow. In this section, we'll tell you how to prepare a framebuffer and make its depth buffer readable. Then, we'll tell you how to render the framebuffer on the screen.

First, depth shouldn't be distorted by projection, which doesn't reflect the physical distance. However, to render it with visible meanings, we'll make it so currently. In the configuration file, `option = ortho` will render it in orthogonal view, other options will be projection view.

## Shaders

The shader itself is dramatically easy:

```glsl
// DepthOnly.vert
#version 330 core

// lightProjectionMat * lightViewMat
uniform mat4 lightSpaceMat;
uniform mat4 modelMat;

layout(location = 0) in vec3 aPosition;

void main()
{
    gl_Position = lightSpaceMat * modelMat * vec4(aPosition, 1.0);    
}
```

Writing positions and the pipeline will automatically fill in depth buffer. The fragment shader just does nothing. Then, to show it on the screen, we assume the depth buffer has been a texture called `diffuseTexture1`(actually not necessarily this name), and write simple shaders to draw it on a quad. Particularly, depth buffer has only one component `r`.

```glsl
// DirectDraw.vert
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
    return;
}

// DirectDraw.frag
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuseTexture1;

void main()
{
    FragColor = vec4(texture(diffuseTexture1, TexCoords).rrr, 1.0);    
    return;
}
```

However, if we want to produce normal visible effects, we need to correct the depth. In graphics, depth output by the fragment will be transformed so that it's more precise when it's close to near plane, and more coarse when it's close to far plane. The transformation is non-linear, so if you use `DirectDraw.frag` directly, you will get a white-dominant result, since most black values are allocated for low depth to give more precision. Thus, if we want to view it normally, we need to correct is back to NDC. In OpenGL, the NDC coordinate is in $[-1,1]$.

```glsl
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuseTexture1;
uniform float near;
uniform float far;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float depth = LinearizeDepth(texture(diffuseTexture1, TexCoords).r);
    FragColor = vec4(vec3(depth / far), 1.0);    
    return;
}
```

We use this shader for projection view and `DirectDraw.frag` for orthogonal view.

## Framebuffer

Recall that we've assume the depth buffer is a texture in the screen shader. This is done by framebuffer; we can render the light space on the framebuffer, and attach its depth buffer to the shader. Particularly, there are two kinds of attachment in framebuffer: renderbuffer and texture. Renderbuffer is only writable, but usually more efficient; texture is readable, so it's more flexible. Here we need to read the depth buffer in another shader, so we need to use it as texture.

+ `Core::Framebuffer` uses configuration type to determine which kind is used. If you provide `Core::TextureParamConfig` defined in `FrameCore/ConfigHelpers/TextureConfig.h`, then it's seen as texture; if it's `Core::RenderBufferConfig` defined in `FrameCore/ConfigHelpers/RenderBufferConfig.h`, then it's seen as renderbuffer. By default, a frame buffer will provide a color texture and a depth renderbuffer. In this section, we doesn't need color texture, and the depth buffer needs to be texture, so we need to adjust it:

  ```c++
  Core::Framebuffer buffer{ width, height, 
      Core::Framebuffer::GetDepthTextureDefaultParamConfig(), {} };
  ```

  We provide `GetDepthTextureDefaultParamConfig` for depth texture, `GetDepthRenderBufferDefaultConfig` for depth renderbuffer(default), and `GetColorTextureDefaultParamConfig` for color texture(default single, here we make it empty by passing `{}`).

  > Note: here we use the width and height of the screen; we can also use other widths & heights, which just makes shadow map more coarse or finer. But remember to use `glViewport(0, 0, framebufferWidth, framebufferHeight)` to adjust it and turn it back to viewport of main window later.

+ We also provide `Model/Mesh` with methods to draw on framebuffer, i.e. designating it as the second parameter, and other parameters are same. For example:

  ```c++
  sucroseModel.Draw(shadowMapShader, buffer);
  ```

+ You can designate how to clear the framebuffer, e.g. `None/DepthClear/ColorClear`. You can use `SetClearMode` to set them so that next `Draw` will clear them. For example:

  ```c++
  using enum Core::Framebuffer::BasicClearMode;
  buffer.SetClearMode({ DepthClear}); // { DepthClear, ColorClear } for multiple clear mode.
  ```

## Special Models

Quad can be directly loaded by `Core::Quad::GetBasicTriRenderModel()` defined in `FrameCore/SpecialModels/SpecialModel.h`. You can also use `GetBasicTriRenderMesh`, `GetBasicTriModel`, `GetBasicTriMesh` to get other types. It's in $[-1,1]^2$; We also provide `Cube` in $[0,1]^3$.

Here, since quad itself is just like a canvas, the depth buffer may not be written for performance:

```c++
glDepthMask(0); // disable
// Draw on quad
glDepthMask(0xFF); // enable
```

## Draw with preprocess

You may notice that quad itself should attach the depth buffer. The textures needed by the model originally will be loaded from file, but depth buffer is attached by ourselves, so we need to do it manaully. This can be done in the preprocess provided by us:

```c++
quadOnScreen.Draw(basicQuadShader, 
                  [&buffer](int textureBeginID, const Core::Shader& shader) {
                      glActiveTexture(GL_TEXTURE0 + textureBeginID);
                      shader.SetInt("diffuseTexture1", textureBeginID);
                      glBindTexture(GL_TEXTURE_2D, buffer.GetDepthBuffer());
                  }, nullptr);
```

The final `nullptr` is to denote that we doesn't need postprocess. Preprocess happens after setting the original textures of the model, and postprocess happens after drawing elements and before unbinding vertex buffers.

For preprocess, the first argument is always `int textureBeginID` and the second is `Shader`. It's needed to activate texture starting from `GL_TEXTURE0 + textureBeginID` and bind the texture. `shader.SetInt` can be used only once without furthur calls if you doesn't change binding between name and texture unit ID, but here we just put it in the loop.

Now, you can fully understand the code in `examples/ShadowMap.cpp`.