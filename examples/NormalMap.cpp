#include "FrameworkCore/Core_All.h"
#include "Utility/IO/IniFile.h"
#include "BasicSettings.h"

using namespace OpenGLFramework;

struct VertAttribWithTan
{
	glm::vec3 normalCoord;
	glm::vec2 textureCoord;
	glm::vec3 tanCoord;
	glm::vec3 bitanCoord;
};

BEGIN_REFLECT(VertAttribWithTan)
REFLECT(1, float, normalCoord)
REFLECT(2, float, textureCoord)
REFLECT(3, float, tanCoord)
REFLECT(4, float, bitanCoord)
END_REFLECT(4)

VERTEX_ATTRIB_SPECIALIZE_COPY(std::vector<VertAttribWithTan>& verticesAttributes_,
	const aiMesh* mesh)
{
	verticesAttributes_.resize(mesh->mNumVertices);
	for (size_t id = 0; id < verticesAttributes_.size(); id++)
	{
		auto& dstVertAttribute = verticesAttributes_[id];
		Core::CopyAiVecToGLMVec(mesh->mNormals[id], dstVertAttribute.normalCoord);

		if (auto srcTextureCoords = mesh->mTextureCoords[0]) [[likely]]
			Core::CopyAiVecToGLMVec(srcTextureCoords[id], dstVertAttribute.textureCoord);
		else
			dstVertAttribute.textureCoord = { 0.f, 0.f };

		Core::CopyAiVecToGLMVec(mesh->mTangents[id], dstVertAttribute.tanCoord);
		Core::CopyAiVecToGLMVec(mesh->mBitangents[id], dstVertAttribute.bitanCoord);
	}
	return;
}

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/NormalMapConfig.ini";
	IOExtension::IniFile file{ configPath };

	bool useOrtho = (file.rootSection("option") == "ortho");
	auto& pathsSection = file.rootSection.GetSubsection("paths")->get();
	auto& windowSection = file.rootSection.GetSubsection("window")->get();
	std::string windowName = windowSection("name");
	unsigned int width = std::stoi(windowSection("width")),
		height = std::stoi(windowSection("height"));

	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	Core::MainWindow mainWindow{ width, height, windowName.c_str() };

	Core::BasicTriRenderModel floor{ pathsSection("plane_resource_dir"),
		std::vector<VertAttribWithTan>{}, true
	};
	floor.transform.scale = { 0.1, 0.1, 0.1 };
	Core::Texture normalMap{ pathsSection("normal_map_dir") };
	Core::Shader normalMapShader{
		pathsSection("vertex_shader_dir"),
		pathsSection("fragment_shader_dir")
	};

	Core::Camera camera{ {0, 5, 0}, {0, 0, -1}, {0, -1, 0} };
	glm::vec3 lightPosition = { 0, 5, 0 };

	mainWindow.Register([&floor, &normalMap, &normalMapShader, &mainWindow,
		&camera, &lightPosition]() 
	{
		normalMapShader.Activate();

		const auto [width, height] = mainWindow.GetWidthAndHeight();
		float near = 0.1f, far = 100.0f;
		SetMVP(width, height, near, far, floor, camera, normalMapShader);

		normalMapShader.SetVec3("lightPos", lightPosition);
		normalMapShader.SetVec3("viewPos", camera.GetPosition());
		floor.Draw(normalMapShader, 
			[&normalMap](int beginID, const Core::Shader& shader) {
				Core::Texture::BindTextureOnShader(beginID, "normalMap",
					shader, normalMap.GetID());
			}, nullptr);
	});
	SetBasicKeyBindings(mainWindow, camera);
	SetBasicTransformSubwindow(mainWindow, floor);
	mainWindow.MainLoop({ 0.0, 0.0, 0.0, 0.0 });
	return 0;
}