#pragma once

#include "FrameworkCore/Framebuffer.h"

class GBuffer
{
	static constexpr size_t c_bufferNum_ = 3;
	static const inline OpenGLFramework::Core::TextureGenConfig c_genConfig_{
		.gpuPixelFormat = OpenGLFramework::GLHelper::ColorInternalFormat<
			OpenGLFramework::GLHelper::ColorComponents::RGBA,
			OpenGLFramework::GLHelper::GPUColorComponentSizeTag::All16,
			OpenGLFramework::GLHelper::GPUComponentType::Float>::safe_value,
		.cpuPixelFormat = OpenGLFramework::Core::TextureGenConfig::CPUPixelFormat::RGBA,
		.rawDataType = OpenGLFramework::GLHelper::ToGLType<float>::safe_value
	};
	static inline const char* names_[c_bufferNum_] = {
		"DirectIllum", "WorldNormal", "WorldPosAndDepth"
	};
public:
	static constexpr size_t GetBufferNum() { return c_bufferNum_; }
	OpenGLFramework::Core::TextureConfigCRef GetConfig() const {
		using namespace OpenGLFramework::Core;
		return { c_genConfig_, Framebuffer::GetColorTextureDefaultConfig().second };
	}
	GBuffer(unsigned int width, unsigned int height) : buffer_{ width, height,
		OpenGLFramework::Core::Framebuffer::GetDepthRenderBufferDefaultConfig(),
		{ GetBufferNum(), GetConfig() }
	} {};

	void ResizeBuffer(unsigned int width, unsigned int height) {
		using namespace OpenGLFramework::Core;
		buffer_ = Framebuffer{ width, height,
			Framebuffer::GetDepthRenderBufferDefaultConfig(),
			{ GetBufferNum(), GetConfig() }
		};
	}
	const auto& GetFrameBuffer() const { return buffer_; }
	void BindAsTextures(int beginID, const Core::Shader& shader) {
		for (unsigned int i = 0; i < GetBufferNum(); i++)
		{
			Core::Texture::BindTextureOnShader(beginID + i, names_[i],
				shader, buffer_.GetColorBuffer(i));
		}
	}

private:
	OpenGLFramework::Core::Framebuffer buffer_;
};