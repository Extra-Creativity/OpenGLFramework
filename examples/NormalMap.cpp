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

class BasicVertAttribContainer : public GLHelper::IVertexAttribContainer
{
public:
	void AllocateAndBind(size_t posSize, size_t vertexNum) final {
		IVertexAttribContainer::AllocateAttributes_(container_, posSize, vertexNum);
		BEGIN_BIND(VertAttribWithTan, posSize, vertexNum);
		BIND_VERTEX_ATTRIB(1, float, normalCoord)
		BIND_VERTEX_ATTRIB(2, float, textureCoord)
		BIND_VERTEX_ATTRIB(3, float, tanCoord)
		BIND_VERTEX_ATTRIB(4, float, bitanCoord)
		END_BIND;
	};

	void CopyFromMesh(const aiMesh* mesh) final 
	{
		container_.resize(mesh->mNumVertices);
		for (size_t id = 0; id < container_.size(); id++)
		{
			auto& dstVertAttribute = container_[id];
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
private:
	std::vector<VertAttribWithTan> container_;
};


int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/NormalMapConfig.ini";
	IOExtension::IniFile file{ configPath };

	auto& pathsSection = file.rootSection.GetSubsection("paths")->get();
	auto& windowSection = file.rootSection.GetSubsection("window")->get();
	std::string windowName = windowSection("name");
	unsigned int width = std::stoi(windowSection("width")),
		height = std::stoi(windowSection("height"));

	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	Core::MainWindow mainWindow{ width, height, windowName.c_str() };
	
	GLHelper::NaiveVACFactory<BasicVertAttribContainer> factory{};
	Core::BasicTriRenderModel floor{ pathsSection("plane_resource_dir"), factory, true };
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
		SetMVP(static_cast<float>(width), static_cast<float>(height),
			near, far, floor, camera, normalMapShader);

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