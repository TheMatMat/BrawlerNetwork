#include <Sel/AnimationSystem.hpp>
#include <Sel/CameraComponent.hpp>
#include <Sel/ComponentRegistry.hpp>
#include <Sel/Core.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/ImGuiRenderer.hpp>
#include <Sel/InputManager.hpp>
#include <Sel/Matrix3.hpp>
#include <Sel/Matrix4.hpp>
#include <Sel/Model.hpp>
#include <Sel/NameComponent.hpp>
#include <Sel/Renderer.hpp>
#include <Sel/RenderSystem.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/Sprite.hpp>
#include <Sel/SpritesheetComponent.hpp>
#include <Sel/Stopwatch.hpp>
#include <Sel/Surface.hpp>
#include <Sel/Texture.hpp>
#include <Sel/Transform.hpp>
#include <Sel/Vector2.hpp>
#include <Sel/VelocityComponent.hpp>
#include <Sel/Window.hpp>
#include <Sel/WorldEditor.hpp>
#include <entt/entt.hpp>
#include <fmt/core.h>
#include <fmt/color.h>
#include <imgui.h>
#include <iostream>

int main(int argc, char* argv[])
{
	Sel::Core core;
	Sel::Window window("SelEngine", 1280, 720);
	Sel::Renderer renderer(window, 1, SDL_RENDERER_PRESENTVSYNC);
	Sel::ResourceManager resourceManager(renderer);
	Sel::ImGuiRenderer imgui(window, renderer);

	ImGui::SetCurrentContext(imgui.GetContext());

	Sel::Stopwatch stopwatch;

	struct Vertex
	{
		Sel::Vector3f position;
		Sel::Color color;
	};

	std::vector<Vertex> vertices{
		{
			{ { -1.f, -1.f, 0.f }, { 1.f, 0.f, 0.f } },
			{ { -1.f,  1.f, 0.f }, { 0.f, 1.f, 0.f } },
			{ {  1.f, -1.f, 0.f }, { 0.f, 0.f, 1.f } },
			{ {  1.f,  1.f, 0.f }, { 1.f, 1.f, 0.f } },
			{ {  1.f,  1.f, -1.f }, { 0.f, 1.f, 1.f } },
			{ {  1.f, -1.f, -1.f }, { 1.f, 0.f, 1.f } },
		}
	};

	std::vector<int> indices{ 0, 1, 2, 1, 2, 3, 2, 3, 5, 3, 4, 5 };

	std::vector<SDL_Vertex> transformedVertices(vertices.size());

	float spriteSize = 250.f;
	Sel::Vector2i windowSize = window.GetSize();

	float cameraXYZ[3] = { 0.f, 0.f, 2.f };

	float timer = 0.f;

	float rotationX = 0.f;
	float rotationY = 0.f;
	float rotationZ = 0.f;
	float fov = 80.f * Sel::Deg2Rad;

	bool isOpen = true;
	bool perspectiveDivide = true;

	while (isOpen)
	{
		float deltaTime = stopwatch.Restart();
		timer += deltaTime;

		SDL_Event event;
		while (Sel::Core::PollEvent(event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					isOpen = false;
					break;
			}

			imgui.ProcessEvent(event);
		}

		// Rendu ici

		renderer.Clear();

		imgui.NewFrame();
		
		Sel::Matrix4f perspectiveMatrix = Sel::Matrix4f::Perspective(fov * Sel::Rad2Deg, (float)windowSize.x / windowSize.y, 1.f, 1000.f);
		Sel::Matrix4f transformMatrix = perspectiveMatrix * Sel::Matrix4f::Translate({cameraXYZ[0], cameraXYZ[1], cameraXYZ[2]}) * Sel::Matrix4f::Translate({ std::sin(timer) * 5.f, 0.f, 0.f }) * Sel::Matrix4f::RotateAroundZ(rotationZ * Sel::Rad2Deg) * Sel::Matrix4f::RotateAroundY(rotationY * Sel::Rad2Deg) * Sel::Matrix4f::RotateAroundX(rotationX * Sel::Rad2Deg);

		for (std::size_t i = 0; i < vertices.size(); ++i)
		{
			Sel::Vector4f position = transformMatrix * Sel::Vector4f(vertices[i].position, 1.f);
			if (perspectiveDivide)
				position /= position.w;

			position += Sel::Vector4f(1.f, 1.f, 0.f, 0.f);
			position.x *= 0.5f;
			position.y *= 0.5f;
			position *= Sel::Vector4f(windowSize.x, windowSize.y, 1.f, 1.f);

			SDL_Vertex& output = transformedVertices[i];
			output.position.x = position.x;
			output.position.y = position.y;

			vertices[i].color.ToRGBA8(output.color.r, output.color.g, output.color.b, output.color.a);
		}

		renderer.RenderGeometry(transformedVertices.data(), transformedVertices.size(), indices.data(), indices.size());

		ImGui::InputFloat3("Camera pos", cameraXYZ);
		ImGui::SliderAngle("Field of View", &fov, 5.f, 180.f);
		ImGui::SliderAngle("Rotation X", &rotationX);
		ImGui::SliderAngle("Rotation Y", &rotationY);
		ImGui::SliderAngle("Rotation Z", &rotationZ);
		ImGui::Checkbox("Perspective divide", &perspectiveDivide);

		imgui.Render(renderer);

		renderer.Present();
	}

	return 0;
}
