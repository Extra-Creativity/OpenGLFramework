#include "BasicSettings.h"

#include "FrameworkCore/Core_All.h"
#include "Utility/IO/IniFile.h"
#include "Utility/IO/IOExtension.h"

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/ShadowMapConfig.ini";
	IOExtension::IniFile file{ configPath };

	bool useOrtho = (file.rootSection("option") == "ortho");
	auto& pathsSection = file.rootSection.GetSubsection("paths")->get();
	auto& windowSection = file.rootSection.GetSubsection("window")->get();
	std::string windowName = windowSection("name");
	unsigned int width = std::stoi(windowSection("width")), 
		height = std::stoi(windowSection("height"));

	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	Core::MainWindow mainWindow{ width, height, windowName.c_str() };

	Core::BasicTriRenderModel sucroseModel{ pathsSection("sucrose_resource_dir")};
	Core::BasicTriRenderModel floor{ pathsSection("plane_resource_dir") };
	Core::Shader shadowMapShader{
		pathsSection("vertex_shader_dir"),
		pathsSection("fragment_shader_dir")
	};
	float near = 0.1f, far = 100.0f;

	Core::Camera lightSpaceCamera{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} };
	lightSpaceCamera.RotateAroundCenter(-15, { 1, 0, 0 }, { 0, 10, 0 });

	glm::mat4 projection = useOrtho ?
		glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near, far) :
		glm::perspective(glm::radians(lightSpaceCamera.fov),
			static_cast<float>(width) / height, near, far);
	glm::mat4 lightSpaceMat = projection * lightSpaceCamera.GetViewMatrix();

	Core::Framebuffer buffer{ width, height, 
		Core::Framebuffer::BasicBufferType::OnlyReadableDepthBuffer };
	
	Core::BasicTriRenderModel quadOnScreen{ pathsSection("quad_resource_dir") };
	mainWindow.Register(
		[&shadowMapShader, &sucroseModel, &floor, &buffer, &lightSpaceMat, 
		 near, far]() {
			shadowMapShader.Activate();
			shadowMapShader.SetMat4("lightSpaceMat", lightSpaceMat);

			shadowMapShader.SetMat4("modelMat", sucroseModel.transform.GetModelMatrix());
			buffer.needDepthClear = true;
			sucroseModel.Draw(shadowMapShader, buffer);

			shadowMapShader.SetMat4("modelMat", floor.transform.GetModelMatrix());
			buffer.needDepthClear = false;
			floor.Draw(shadowMapShader, buffer);
		});

	Core::Shader basicQuadShader{
		pathsSection("quad_vertex_shader_dir"),
		useOrtho ? pathsSection("quad_ortho_fragment_shader_dir") : 
				pathsSection("quad_persp_fragment_shader_dir")
	};
	mainWindow.Register([&quadOnScreen, &buffer, &basicQuadShader, useOrtho, near, far]() {
		glDepthMask(0);
		basicQuadShader.Activate();
		if (!useOrtho) {
			basicQuadShader.SetFloat("near", near);
			basicQuadShader.SetFloat("far", far);
		}

		quadOnScreen.Draw(basicQuadShader, 
			[&buffer](int textureBeginID, Core::Shader& shader) {
			glActiveTexture(GL_TEXTURE0 + textureBeginID);
			shader.SetInt("diffuseTexture1", textureBeginID);
			glBindTexture(GL_TEXTURE_2D, buffer.GetDepthBuffer());
		}, nullptr);
		glDepthMask(0xFF);
	});
	mainWindow.MainLoop({ 0.0, 0.0, 0.0, 0.0 });
    return 0;
}