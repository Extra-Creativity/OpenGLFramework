#include "PrepareContext.h"
#include "MainWindow.h"
#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#include "Framebuffer.h"

#include <imgui.h>

#include <iostream>
#include <chrono>
#include <numbers>

#include "config.h"

// Possible optimization : 
// MainWindow : std::unordered_map can be changed to std::vector
// when the element amount is small, the latter is faster.

int main()
{
	// auto t1 = std::chrono::steady_clock::now(), t2 = t1;
	InitContext();
	MainWindow mainWindow { 800, 600, "OpenGL Test"};
	Model model{ OPENGLFRAMEWORK_RESOURCE_DIR "Sucrose/Sucrose.pmx"};
	Shader shader{ OPENGLFRAMEWORK_SHADER_DIR "Sucrose.vert", OPENGLFRAMEWORK_SHADER_DIR "Sucrose.frag" };
	Camera frontCamera{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} };
	// t2 = std::chrono::steady_clock::now();
	// std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();

	Camera sideCamera{ {-30, 10, 18}, {0, 1, 0}, {30, 0, -18} };
	Framebuffer frameBuffer;

	float near = 0.1f, far = 100.0f;
	//mainWindow.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	mainWindow.Register([&shader, &model, &frontCamera, &sideCamera, &mainWindow, &frameBuffer, near, far]() {
		glEnable(GL_DEPTH_TEST);
		shader.Activate();
		auto modelMat = model.transform.GetModelMatrix();
		shader.SetMat4("model", modelMat);

		auto viewMat = frontCamera.GetViewMatrix();
		shader.SetMat4("view", viewMat);

		const auto [width, height] = mainWindow.GetWidthAndHeight();

		auto projectionMat = glm::perspective(glm::radians(frontCamera.fov), static_cast<float>(width) / height, near, far);
		shader.SetMat4("projection", projectionMat);

		model.Draw(shader);

		ImGui::Begin("Back");
		static bool init = true;
		if (init)
		{
			ImGui::SetWindowPos({ 50, 200 });
			ImGui::SetWindowSize({ 200, 350 });
			init = false;
		}

		[[maybe_unused]] const auto& io = ImGui::GetIO();

		auto subwindowSize = ImGui::GetWindowSize();

		if ((subwindowSize.x != frameBuffer.GetWidth() || subwindowSize.y != frameBuffer.GetHeight()) && 
			!ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			glViewport(0, 0, static_cast<size_t>(subwindowSize.x), static_cast<size_t>(subwindowSize.y));

			frameBuffer.resize(static_cast<size_t>(subwindowSize.x), static_cast<size_t>(subwindowSize.y));

			viewMat = sideCamera.GetViewMatrix();
			shader.SetMat4("view", viewMat);

			projectionMat = glm::perspective(glm::radians(frontCamera.fov), subwindowSize.x / subwindowSize.y, near, far);
			shader.SetMat4("projection", projectionMat);

			model.Draw(shader, frameBuffer);
		}

		ImGui::Image(reinterpret_cast<ImTextureID>(frameBuffer.textureColorBuffer), subwindowSize, { 0, 1 }, { 1, 0 });
		ImGui::End();
	});

	mainWindow.Register([&model]() {
		static bool init = true;
		ImGui::Begin("Model Adjustment");
		if (init)
		{
			ImGui::SetWindowPos({ 50, 50 });
			ImGui::SetWindowSize({ 250,100 });
			init = false;
		}
		static glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		ImGui::SliderFloat3("rotation", &rotation.x, 0.0f, 2 * std::numbers::pi_v<float>);
		model.transform.rotation = glm::quat(rotation);

		static glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		ImGui::SliderFloat3("position", &position.x, -10, 10);
		model.transform.position = position;

		static float scale = 1.0f;
		ImGui::SliderFloat("scale", &scale, 0.5, 2);
		model.transform.scale = { scale,scale,scale };
		ImGui::End();
	});

	mainWindow.BindScrollCallback([&frontCamera](double, double yOffset) {
		float afterFov = frontCamera.fov - static_cast<float>(yOffset);
		frontCamera.fov = glm::clamp(afterFov, 15.0f, 75.0f);
	});

	// NOTE that code below(glfw) will reset the existing imgui handle, so io.WantCaptureMouse will always
	// be false. See https://stackoverflow.com/a/72509936/15582103 for more info.
	// 
	//mainWindow.BindCursorPosCallback([&frontCamera](double xPos, double yPos) {
	//	static float lastxPos = static_cast<float>(xPos), lastyPos = static_cast<float>(yPos);
	//	float xOffset = lastxPos - static_cast<float>(xPos), yOffset = lastyPos - static_cast<float>(yPos);
	//	lastxPos = static_cast<float>(xPos), lastyPos = static_cast<float>(yPos);
	//	frontCamera.Rotate(static_cast<float>(xOffset) * frontCamera.mouseSensitivity, { 0, 1, 0 });
	//	frontCamera.Rotate(static_cast<float>(yOffset) * frontCamera.mouseSensitivity, { 1, 0, 0 });
	//});

	mainWindow.BindKeyPressing<GLFW_KEY_W>([&frontCamera, &mainWindow]() {
		frontCamera.Translate(mainWindow.deltaTime * glm::vec3{ 0.0f, 0.0f, -frontCamera.movementSpeed });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_S>([&frontCamera, &mainWindow]() {
		frontCamera.Translate(mainWindow.deltaTime * glm::vec3{ 0.0f, 0.0f, frontCamera.movementSpeed });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_A>([&frontCamera, &mainWindow]() {
		frontCamera.Translate(mainWindow.deltaTime * glm::vec3{ -frontCamera.movementSpeed, 0.0f, 0.0f });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_D>([&frontCamera, &mainWindow]() {
		frontCamera.Translate(mainWindow.deltaTime * glm::vec3{ frontCamera.movementSpeed, 0.0f, 0.5f });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_UP>([&frontCamera, &mainWindow]() {
		frontCamera.Rotate(mainWindow.deltaTime * frontCamera.rotationSpeed, { 1, 0, 0 });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_DOWN>([&frontCamera, &mainWindow]() {
		frontCamera.Rotate(-mainWindow.deltaTime * frontCamera.rotationSpeed, { 1, 0, 0 });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_LEFT>([&frontCamera, &mainWindow]() {
		frontCamera.Rotate(mainWindow.deltaTime * frontCamera.rotationSpeed, { 0, 1, 0 });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_RIGHT>([&frontCamera, &mainWindow]() {
		frontCamera.Rotate(-mainWindow.deltaTime * frontCamera.rotationSpeed, { 0, 1, 0 });
	});

	mainWindow.BindKeyPressed<GLFW_KEY_SPACE>([&frontCamera]() {
		std::cout << "Pressed\n";
		frontCamera.Translate({ 0, 1.0f, 0.0f });
	});
	mainWindow.BindKeyReleased<GLFW_KEY_SPACE>([&frontCamera]() {
		std::cout << "Released\n";
		frontCamera.Translate({ 0, -1.0f, 0.0f });
	});
	mainWindow.BindKeyPressed<GLFW_KEY_ESCAPE>([&mainWindow]() {
		mainWindow.Close();
	});
	mainWindow.MainLoop({ 1.0f, 1.0f, 1.0f, 1.0f });
	EndContext();
	return 0;
}

