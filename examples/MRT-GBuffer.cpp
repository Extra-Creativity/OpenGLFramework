#include "BasicSettings.h"

#include "FrameworkCore/Core_All.h"
#include "Utility/IO/IniFile.h"

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/MRT-GBufferConfig.ini";
	IOExtension::IniFile file{ configPath };

	auto& pathsSection = file.rootSection.GetSubsection("paths")->get();
	auto& windowSection = file.rootSection.GetSubsection("window")->get();
	std::string windowName = windowSection("name");
	unsigned int width = std::stoi(windowSection("width")),
		height = std::stoi(windowSection("height"));

	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	Core::MainWindow mainWindow{ width, height, windowName.c_str() };

	Core::BasicTriRenderModel sucroseModel{ pathsSection("sucrose") };
	Core::BasicTriRenderModel floorModel{ pathsSection("floor") };

	Core::Shader MRTShader{
		pathsSection("mrt_vertex_shader"),
		pathsSection("mrt_fragment_shader")
	};

	Core::Camera frontCamera{ { 0, 10, 35}, {0, 1, 0}, {0, 0, -1} };
	Core::RenderBufferConfig config{
		.bufferType = GLHelper::ColorInternalFormat<GLHelper::ColorComponents::RGBA,
			GLHelper::GPUColorComponentSizeTag::All8>::safe_value,
		.attachmentType = Core::RenderBufferConfig::AttachmentType::Color
	};
	std::vector<Core::Framebuffer::ConfigType> vec(4, config);

	Core::Framebuffer buffer{ width, height,
		Core::Framebuffer::GetDepthRenderBufferDefaultConfig(), vec	};

	mainWindow.Register([&]() {
		using enum Core::Framebuffer::BasicClearMode;
		buffer.Clear();

		MRTShader.Activate();

		const auto [width, height] = mainWindow.GetWidthAndHeight();
		float near = 0.1f, far = 100.0f;
		SetMVP(static_cast<float>(width), static_cast<float>(height),
			near, far, sucroseModel, frontCamera, MRTShader);
		sucroseModel.Draw(MRTShader, buffer);
		
		SetMVP(static_cast<float>(width), static_cast<float>(height),
			near, far, floorModel, frontCamera, MRTShader);
		floorModel.Draw(MRTShader, buffer);
	});

	mainWindow.Register([&]() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer.GetFramebuffer());
		const auto [width, height] = mainWindow.GetWidthAndHeight();
		const auto bufferWidth = buffer.GetWidth(), 
			bufferHeight = buffer.GetHeight();

		// Transferring depth buffer usually uses this way, but we need to visualize
		// it, so here we don't do so. This will just set another depth buffer,
		// which can affect future depth testing.
		// glBlitFramebuffer(0, 0, bufferWidth, bufferHeight,
		//	 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_LINEAR);

		// left-bottom(pos) -> right-bottom(normal) -> left-top(color) -> right-top(depth)
		const std::pair<int, int> beginCoords[]{ { 0, 0 }, {width / 2, 0},
			{0, height / 2}, {width / 2, height / 2} };

		for (int i = 0; i < std::size(beginCoords); i++)
		{
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
			glBlitFramebuffer(0, 0, buffer.GetWidth(), buffer.GetHeight(),
				beginCoords[i].first, beginCoords[i].second, 
				beginCoords[i].first + width / 2, beginCoords[i].second + height / 2,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});

	mainWindow.MainLoop({ 0.0, 0.0, 0.0, 0.0 });
	return 0;
}