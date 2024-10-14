#include <Sel/AnimationSystem.hpp>
#include <Sel/CameraComponent.hpp>
#include <Sel/ComponentRegistry.hpp>
#include <Sel/Core.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/ImGuiRenderer.hpp>
#include <Sel/InputManager.hpp>
#include <Sel/Matrix3.hpp>
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

struct InputComponent
{
	bool left = false;
	bool right = false;
	bool jump = false;

	void PopulateInspector(const Sel::WorldEditor& worldEditor)
	{
		ImGui::Checkbox("Left", &left);
		ImGui::Checkbox("Right", &right);
		ImGui::Checkbox("Jump", &jump);
	}
};

std::shared_ptr<Sel::Sprite> BuildRunnerSprite(Sel::Renderer& renderer)
{
	std::shared_ptr<Sel::Texture> runnerTexture = Sel::ResourceManager::Instance().GetTexture("assets/runner.png");
	std::shared_ptr<Sel::Sprite> runnerSprite = std::make_shared<Sel::Sprite>(runnerTexture, SDL_Rect{ 0, 0, 32, 32 });
	runnerSprite->Resize(256, 256);

	return runnerSprite;
}

entt::handle CreateModel(entt::registry& registry, std::shared_ptr<Sel::Renderable> renderable, const Sel::Vector2f& position)
{
	entt::entity entity = registry.create();

	auto& transform = registry.emplace<Sel::Transform>(entity);
	transform.SetPosition(position);

	auto& gfx = registry.emplace<Sel::GraphicsComponent>(entity);
	gfx.renderable = std::move(renderable);

	return entt::handle{ registry, entity };
}

entt::handle CreateRunner(entt::registry& registry, std::shared_ptr<Sel::Sprite> sprite, std::shared_ptr<Sel::Spritesheet> spritesheet, const Sel::Vector2f& position)
{
	entt::entity entity = registry.create();

	auto& transform = registry.emplace<Sel::Transform>(entity);
	transform.SetPosition(position);

	auto& gfx = registry.emplace<Sel::GraphicsComponent>(entity);
	gfx.renderable = sprite;

	auto& spritesheetComponent = registry.emplace<Sel::SpritesheetComponent>(entity, spritesheet, sprite);

	auto& velocity = registry.emplace<Sel::VelocityComponent>(entity);
	velocity.linearVel = Sel::Vector2f{ 0.f, 0.f };

	return entt::handle{ registry, entity };
}

void GravitySystem(entt::registry& registry, float deltaTime)
{
	auto view = registry.view<Sel::VelocityComponent>();
	for (auto&& [entity, velocity] : view.each())
		velocity.linearVel.y += 1000.f * deltaTime;
}

void InputSystem(entt::registry& registry)
{
	auto& inputManager = Sel::InputManager::Instance();

	auto view = registry.view<InputComponent>();
	for (auto&& [entity, inputs] : view.each())
	{
		inputs.left = inputManager.IsActive("MoveLeft");
		inputs.right = inputManager.IsActive("MoveRight");
	}
}

void VelocitySystem(entt::registry& registry, float deltaTime)
{
	auto view = registry.view<Sel::Transform, Sel::VelocityComponent>();
	for (auto&& [entity, transform, velocity] : view.each())
	{
		transform.Translate(velocity.linearVel * deltaTime);
		//transform.Rotate(deltaTime * 10.f);

		Sel::Vector2f pos = transform.GetPosition();
		if (pos.y > 720 - 128)
		{
			pos.y = 720 - 128;
			transform.SetPosition(pos);
			velocity.linearVel.y = 0.f;
		}
	}
}

void PlayerControllerSystem(entt::registry& registry)
{
	auto view = registry.view<InputComponent, Sel::VelocityComponent>();
	for (auto&& [entity, input, velocity] : view.each())
	{
		velocity.linearVel.x = 0.f;

		if (input.left)
			velocity.linearVel.x -= 1000.f;

		if (input.right)
			velocity.linearVel.x += 1000.f;
	}
}

entt::handle GetHoveredEntity(entt::registry& registry, entt::handle camera)
{
	Sel::Vector2i mousePos = Sel::Core::GetMousePosition();
	Sel::Vector2f relativePos(mousePos.x, mousePos.y);

	auto& cameraTransform = camera.get<Sel::Transform>();

	auto view = registry.view<Sel::Transform, Sel::GraphicsComponent>();
	for (auto&& [entity, transform, gfx] : view.each())
	{
		Sel::Vector2f worldPos = cameraTransform.TransformPoint(relativePos);
		Sel::Vector2f localPos = transform.TransformInversePoint(worldPos);

		SDL_FRect bounds = gfx.renderable->GetBounds();
		if (localPos.x >= bounds.x && localPos.y >= bounds.y &&
			localPos.x < bounds.x + bounds.w && localPos.y < bounds.y + bounds.h)
		{
			return entt::handle(registry, entity);
		}
	}

	return entt::handle{};
}

int main(int argc, char* argv[])
{
	Sel::Core core;
	Sel::Window window("SelEngine", 1280, 720);
	Sel::Renderer renderer(window);
	Sel::ResourceManager resourceManager(renderer);
	Sel::InputManager inputManager;
	Sel::ImGuiRenderer imgui(window, renderer);

	// Si on initialise ImGui dans une DLL (ce que nous faisons avec la classe ImGuiRenderer) et l'utilisons dans un autre exécutable (DLL/.exe)
	// la bibliothèque nous demande d'appeler ImGui::SetCurrentContext dans l'exécutable souhaitant utiliser ImGui, avec le contexte précédemment récupéré
	// Ceci est parce qu'ImGui utilise des variables globales en interne qui ne sont pas partagées entre la .dll et l'exécutable (comme indiqué dans sa documentation)
	ImGui::SetCurrentContext(imgui.GetContext());

	renderer.SetDrawColor(0, 0, 0);
	
	entt::registry world;

	Sel::AnimationSystem animationSystem(world);
	Sel::RenderSystem renderSystem(renderer, world);

	std::shared_ptr<Sel::Spritesheet> runnerSpritesheet = resourceManager.GetSpritesheet("assets/runner.spritesheet");

	entt::handle runner = CreateRunner(world, BuildRunnerSprite(renderer), runnerSpritesheet, Sel::Vector2f { 200.f, 200.f });
	runner.emplace<Sel::NameComponent>("Player entity");

	entt::handle runner2 = CreateRunner(world, BuildRunnerSprite(renderer), runnerSpritesheet, Sel::Vector2f { 400.f, 200.f });
	runner2.emplace<Sel::NameComponent>("Runner2");

	std::shared_ptr<Sel::Model> houseModel = resourceManager.GetModel("assets/house.bmodel");

	entt::handle houseEntity = CreateModel(world, houseModel, Sel::Vector2f { 800.f, 600.f });

	world.emplace<InputComponent>(runner);

	Sel::Stopwatch stopwatch;
	
	inputManager.BindKeyPressed(SDL_KeyCode::SDLK_F1, "OpenEditor");
	inputManager.BindKeyPressed(SDL_KeyCode::SDLK_LEFT, "MoveLeft");
	inputManager.BindKeyPressed(SDL_KeyCode::SDLK_RIGHT, "MoveRight");
	inputManager.BindMouseButtonPressed(Sel::MouseButton::Left, "Inspect");

	entt::handle camera = { world, world.create() };
	camera.emplace<Sel::Transform>().SetPosition({ -200.f, 300.f });
	camera.emplace<Sel::CameraComponent>();
	camera.emplace<Sel::NameComponent>("Camera");

	Sel::ComponentRegistry componentRegistry;
	componentRegistry.Register({
		.id = "inputs",
		.label = "Inputs",
		.addComponent = Sel::ComponentRegistry::BuildAddComponent<InputComponent>(),
		.hasComponent = Sel::ComponentRegistry::BuildHasComponent<InputComponent>(),
		.removeComponent = Sel::ComponentRegistry::BuildRemoveComponent<InputComponent>(),
		.inspect = Sel::ComponentRegistry::BuildInspect<InputComponent>()
	});

	std::optional<Sel::WorldEditor> worldEditor;
	inputManager.BindAction("OpenEditor", [&](bool active)
	{
		if (!active)
			return;

		if (worldEditor)
			worldEditor.reset();
		else
			worldEditor.emplace(window, world, componentRegistry);
	});

	bool isOpen = true;
	while (isOpen)
	{
		float deltaTime = stopwatch.Restart();

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

			inputManager.HandleEvent(event);
		}

		// Rendu ici

		renderer.Clear();

		imgui.NewFrame();

		if (!worldEditor || !worldEditor->IsPaused())
		{
			PlayerControllerSystem(world);
			GravitySystem(world, deltaTime);
			VelocitySystem(world, deltaTime);
			InputSystem(world);
			animationSystem.Update(deltaTime);
		}

		renderSystem.Update(deltaTime);

		if (worldEditor)
			worldEditor->Render();

		imgui.Render(renderer);

		renderer.Present();
	}

	return 0;
}
