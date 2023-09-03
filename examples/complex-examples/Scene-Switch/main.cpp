#include "../Base/BasicSettings.h"
#include "../Base/AssetLoader.h"

#include "FrameworkCore/Core_All.h"

int main()
{
	[[maybe_unused]] auto& manager = Core::ContextManager::GetInstance();
	ExampleBase::AssetLoader loader{
		std::filesystem::path{ CONFIG_DIR } / "config.ini",
		std::filesystem::path{ MODEL_DIR },
		std::filesystem::path{ SHADER_DIR }
	};
	auto& mainWindow = loader.GetMainWindowInstance();
	Core::Texture icons{ std::filesystem::path{ MODEL_DIR } / "Icons/icons.png" };
	auto quad = Core::Quad::GetBasicTriRenderMesh();

	mainWindow.Register([&loader, &quad, &icons, &mainWindow]() {
		static bool needInit = true;
		if (needInit) {
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF); // 所有的片段都应该更新模板缓冲
			glStencilMask(0xFF);

			float iconsAspect;
			{
				const auto [width, height] = icons.GetWidthAndHeight();
				iconsAspect = static_cast<float>(width) / height;
			}
			auto& shader = loader.GetShader("stencil");
			shader.Activate();
			shader.SetFloat("aspect", iconsAspect / mainWindow.GetAspect());
			quad.Draw(shader, [&icons](int beginID, const Core::Shader& shader) {
				Core::Texture::BindTextureOnShader(beginID, "icons", shader, icons.GetID());
			}, nullptr);
			needInit = false;
			glStencilMask(0);
		}
	});

	mainWindow.Register([&loader, &quad, &icons] {
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		static int cnt = -100;
		cnt++;
		auto& shader = loader.GetShader("screen");
		shader.Activate();
		shader.SetFloat("border", std::clamp(cnt / 1000.0f, 0.0f, 1.0f));
		quad.Draw(shader);
	});

	mainWindow.MainLoop({ 1.0,1.0,1.0,1.0 });
    return 0;
}