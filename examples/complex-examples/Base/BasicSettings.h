#include "FrameworkCore/MainWindow.h"
#include "FrameworkCore/Shader.h"
#include "FrameworkCore/Camera.h"

#include <numbers>

#include <imgui.h>

using namespace OpenGLFramework;

template<typename Model>
void SetMVP(float width, float height, float near, float far, 
	Model& model, Core::Camera& camera, Core::Shader& shader)
{
	auto modelMat = model.transform.GetModelMatrix();
	shader.SetMat4("model", modelMat);

	auto viewMat = camera.GetViewMatrix();
	shader.SetMat4("view", viewMat);

	auto projectionMat = glm::perspective(glm::radians(camera.fov), 
		width / height, near, far);
	shader.SetMat4("projection", projectionMat);

	return;
}

inline void SetBasicKeyBindings(Core::MainWindow& mainWindow, Core::Camera& camera)
{
    mainWindow.BindKeyPressing<GLFW_KEY_W>([&camera, &mainWindow]() {
		camera.Translate(mainWindow.GetDeltaTime() *
			camera.movementSpeed * camera.Front());
	});
	mainWindow.BindKeyPressing<GLFW_KEY_S>([&camera, &mainWindow]() {
		camera.Translate(mainWindow.GetDeltaTime() * 
			camera.movementSpeed * camera.Back());
	});
	mainWindow.BindKeyPressing<GLFW_KEY_A>([&camera, &mainWindow]() {
		camera.Translate(mainWindow.GetDeltaTime() *
			camera.movementSpeed * camera.Left());
	});
	mainWindow.BindKeyPressing<GLFW_KEY_D>([&camera, &mainWindow]() {
		camera.Translate(mainWindow.GetDeltaTime() *
			camera.movementSpeed * camera.Right());
	});
	mainWindow.BindKeyPressing<GLFW_KEY_Q>([&camera, &mainWindow]() {
		camera.Translate(mainWindow.GetDeltaTime() *
			camera.movementSpeed * camera.Up());
	});
	mainWindow.BindKeyPressing<GLFW_KEY_E>([&camera, &mainWindow]() {
		camera.Translate(mainWindow.GetDeltaTime() *
			camera.movementSpeed * camera.Down());
	});

	mainWindow.BindKeyPressing<GLFW_KEY_UP>([&camera, &mainWindow]() {
		camera.Rotate(
			mainWindow.GetDeltaTime() * camera.rotationSpeed, { 1, 0, 0 });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_DOWN>([&camera, &mainWindow]() {
		camera.Rotate(
			-mainWindow.GetDeltaTime() * camera.rotationSpeed, { 1, 0, 0 });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_LEFT>([&camera, &mainWindow]() {
		camera.Rotate(
			mainWindow.GetDeltaTime() * camera.rotationSpeed, { 0, 1, 0 });
	});
	mainWindow.BindKeyPressing<GLFW_KEY_RIGHT>([&camera, &mainWindow]() {
		camera.Rotate(
			-mainWindow.GetDeltaTime() * camera.rotationSpeed, { 0, 1, 0 });
	});

	mainWindow.BindKeyPressed<GLFW_KEY_ESCAPE>([&mainWindow]() {
		mainWindow.Close();
	});
	mainWindow.BindKeyPressed<GLFW_KEY_R>([camera, &newCamera = camera]() {
		newCamera = camera;
	});
}

inline void SetBasicButtonBindings(Core::MainWindow& mainWindow, Core::Camera& camera)
{
	mainWindow.BindScrollCallback([&camera](double, double yOffset) {
		float afterFov = camera.fov - static_cast<float>(yOffset);
		camera.fov = glm::clamp(afterFov, 15.0f, 75.0f);
	});

	static bool lastNotDrag = false;
	static float initCameraY = camera.GetPosition().y;
	mainWindow.BindMouseButtonPressing<GLFW_MOUSE_BUTTON_RIGHT>([&mainWindow, &camera]() {
		const auto [xPos, yPos] = mainWindow.GetCursorPos();
		static float lastxPos = xPos, lastyPos = yPos;
		float xOffset = lastxPos - xPos, yOffset = lastyPos - yPos;
		lastxPos = xPos, lastyPos = yPos;
		if (lastNotDrag)
		{
			lastNotDrag = false;
			return;
		}

		camera.RotateAroundCenter(camera.mouseSensitivity * xOffset, { 0, 1, 0 },
			{ 0, initCameraY, 0 });

		const auto& gaze = camera.GetGaze();
		if (1 - std::abs(gaze.y) < 1e-1f) // enough for most cases.
			camera.RotateAroundCenter(camera.mouseSensitivity * yOffset, camera.Right(),
				{ 0, initCameraY, 0 });
		else
		{
			glm::vec3 yAxis = glm::cross(gaze, { 0, 1, 0 });
			camera.RotateAroundCenter(camera.mouseSensitivity * yOffset, yAxis,
				{ 0, initCameraY, 0 });
		}
	});
	mainWindow.BindMouseButtonReleasing<GLFW_MOUSE_BUTTON_RIGHT>([]() { lastNotDrag = true; });
}

template<typename Model>
void SetBasicTransformSubwindow(Core::MainWindow& mainWindow, Model& model)
{
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
		}
	);
}