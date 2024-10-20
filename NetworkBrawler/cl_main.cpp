#pragma once

#include "sh_constants.h"
#include "enet6/enet.h"
#include "entt/entt.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <Sel/Color.hpp>
#include <Sel/AnimationSystem.hpp>
#include <Sel/CameraComponent.hpp>
#include <Sel/ComponentRegistry.hpp>
#include <Sel/Stopwatch.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/Font.hpp>
#include <Sel/InputManager.hpp>
#include <Sel/Model.hpp>
#include <Sel/PhysicsSystem.hpp>
#include <Sel/RenderSystem.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/RigidBodyComponent.hpp>
#include <Sel/Core.hpp>
#include <Sel/ImGuiRenderer.hpp>
#include <Sel/Renderer.hpp>
#include <Sel/Texture.hpp>
#include <Sel/Window.hpp>
#include <Sel/Sprite.hpp>
#include <Sel/SpritesheetComponent.hpp>
#include <Sel/Transform.hpp>
#include <Sel/VelocityComponent.hpp>
#include <Sel/VelocitySystem.hpp>
#include <Sel/WorldEditor.hpp>
#include <imgui.h>
#include "sh_inputs.h"
#include "sh_brawler.h"
#include "cl_brawler.h"
#include "sv_networkedcomponent.h"
#include "cl_FloatingEntitySystem.h"
#include "cl_uiFlags.h"
#include "sh_temporaryEntitySystem.h"

struct PlayerData
{
	std::string name;
	std::optional<std::uint32_t> ownBrawlerId;
	bool isDead = true;
};

struct ClientNetworkId
{
	std::uint32_t networkId;
};

struct GameData
{
	Sel::Stopwatch clock;
	float nextTick = 0.f;
	float tickInterval = TickDelay;

	bool isReady = false;

	GameState gameState;
	PlayerMode playerMode;
	std::size_t spectateIndex = 0;
	std::size_t previousSpectateIndex = 1;
	std::uint8_t playerScore; 

	float nextKillTimer = 10.f;

	float waitBeforeSpectate = 1.5f;
	Sel::Stopwatch beforeSpectateClock;

	float stealCooldown = 1.5f;
	Sel::Stopwatch timeSinceLastSteal;

	std::string name;
	std::map<std::uint32_t, PlayerData> players;
	std::map<std::uint32_t, PlayerData> spectatablePlayers;
	ENetPeer* serverPeer; //< Le serveur
	entt::registry* registry;
	entt::registry* registryUI;
	Sel::Renderer* renderer;
	FloatingEntitySystem* floatingEntitySystem;
	FloatingEntitySystem* floatingEntitySystemUI;
	TemporaryEntitySystem* temporaryEntitySystem;
	TemporaryEntitySystem* temporaryEntitySystemUI;
	std::unordered_map<std::uint32_t /*networkId*/, entt::handle> networkToEntities; //< toutes les entités (ici tous les brawlers)
	PlayerInputs inputs; //< Les inputs du joueur
	std::size_t ownPlayerIndex; //< Notre propre ID
	std::optional<std::uint32_t> ownBrawlerNetworkIndex; //< l'ID reseau de notre brawler

	float interpolationFactor = 0.f;
	std::vector<BrawlerStatesPacket> snapshots;
};

void handle_message(const std::vector<std::uint8_t>& message, GameData& gameData);
bool run_network(ENetHost* host, GameData& gameData);
void tick(GameData& gameData);

entt::handle CreateCamera(entt::registry& registry);
Sel::Sprite BuildCollectibleSprite(float size, const CollectibleType& type);
entt::handle SpawnCollectible(GameData& gameData, const CreateCollectiblePacket& packet);
entt::handle CreateDisplayText(GameData& gameData, Sel::Renderer& renderer, std::string text, int fontSize, const Sel::Color& textColor, const std::string& fontPath, Sel::Vector2f origin = {0.5f, 0.5f}, bool isUI = true);
void OneShotAnimationSystem(GameData& gameData, float deltaTime);

int main()
{
	GameData gameData;
	if (enet_initialize() != 0)
	{
		std::cout << "Failed to initialize ENet" << std::endl;
		return EXIT_FAILURE;
	}


	// On demande au joueur de rentrer un nom
	std::string name;
	do
	{
		std::cout << "Please enter a name: " << std::flush;
		std::getline(std::cin, name);
	} while (name.empty());

	ENetHost* host = nullptr;

	// Demande d'une adresse IP et connexion au serveur
	gameData.serverPeer = nullptr;
	for (;;)
	{
		std::string ipAddress;
		std::cout << "Please enter server address: " << std::flush;
		std::cin >> ipAddress;

		ENetAddress serverAddress;
		if (enet_address_set_host(&serverAddress, ENET_ADDRESS_TYPE_ANY, ipAddress.data()) != 0)
		{
			std::cout << "Failed to resolve address" << std::endl;
			continue;
		}
		serverAddress.port = AppPort;

		std::cout << "Connecting..." << std::endl;

		// On ne créé l'host qu'une fois qu'on connait le type d'adresse (IPv4/IPv6)
		if (host)
			enet_host_destroy(host);

		host = enet_host_create(serverAddress.type, nullptr, 1, 0, 0, 0);
		if (!host)
		{
			std::cout << "Failed to initialize host" << std::endl;
			return EXIT_FAILURE;
		}

		// On tente une connexion...
		gameData.serverPeer = enet_host_connect(host, &serverAddress, 0, 0);
		assert(gameData.serverPeer);
		{
			// On utilise la fonction enet_host_service avant d'entrer dans la boucle pour valider la connexion
			// Note : il est important d'appeler plusieurs fois la fonction plutôt qu'une seule fois dans ce cas, pour autoriser plusieurs tentatives de connexion
			// (l'API ENet ayant été conçue autour du fait qu'enet_host_service était appelé en boucle)

			// Nous simulons ça avec une boucle, où chaque tour de boucle va attendre 100ms (pour un total de 5s)
			for (std::size_t i = 0; i < 50; ++i)
			{
				ENetEvent event;
				if (enet_host_service(host, &event, 100) > 0)
				{
					// Nous avons un événement, la connexion a soit pu s'effectuer (ENET_EVENT_TYPE_CONNECT) soit échoué (ENET_EVENT_TYPE_DISCONNECT)
					break; //< On sort de la boucle
				}
			}

			// On vérifie après la boucle l'état du peer, sommes-nous connectés ?
			if (gameData.serverPeer->state != ENET_PEER_STATE_CONNECTED)
			{
				// On force la réinitialisation du serveur auprès d'enet pour pouvoir allouer un nouveau peer
				enet_peer_reset(gameData.serverPeer);

				std::cout << "failed to connect." << std::endl;
				continue;
			}

			// Si on arrive ici, on est connecté !
			std::cout << "Connected!" << std::endl;
			break; //< On casse la boucle demandant une IP pour se connecter
		}

		break;
	}



	Sel::Core core;

	Sel::Window window("Braaaaaawl", WINDOW_WIDTH, WINDOW_HEIGHT);
	Sel::Renderer renderer(window, SDL_RENDERER_PRESENTVSYNC);
	gameData.renderer = &renderer;

	Sel::ResourceManager resourceManager(renderer);
	Sel::InputManager inputManager;

	Sel::ImGuiRenderer imgui(window, renderer);
	ImGui::SetCurrentContext(imgui.GetContext());

	entt::registry registry;
	entt::registry registryUI;

	/// ============ Animation & Rendering ============
	Sel::RenderSystem renderSystem(renderer, registry);
	Sel::RenderSystem renderSystemUI(renderer, registryUI);

	Sel::AnimationSystem animationSystem(registry);
	

	Sel::PhysicsSystem physicsSystem(registry);
	physicsSystem.SetGravity({ 0.f, 0.f });
	physicsSystem.SetDamping(0.9f);

	Sel::VelocitySystem velocitySystem(registry);

	FloatingEntitySystem floatingEntitySystem(&registry);
	gameData.floatingEntitySystem = &floatingEntitySystem;
	
	FloatingEntitySystem floatingEntitySystemUI(&registryUI);
	gameData.floatingEntitySystemUI = &floatingEntitySystemUI;

	TemporaryEntitySystem temporaryEntitySystem(registry);
	gameData.temporaryEntitySystem = &temporaryEntitySystem;
	
	TemporaryEntitySystem temporaryEntitySystemUI(registryUI);
	gameData.temporaryEntitySystemUI = &temporaryEntitySystemUI;

	gameData.registry = &registry;
	gameData.registryUI = &registryUI;

	gameData.gameState = GameState::Lobby;
	gameData.playerMode = PlayerMode::Pending;

	Sel::ComponentRegistry componentRegistry;

	inputManager.BindKeyPressed(SDL_KeyCode::SDLK_F1, "OpenEditor");

	std::optional<Sel::WorldEditor> worldEditor;
	inputManager.BindAction("OpenEditor", [&](bool active)
		{
			if (!active)
				return;

			if (worldEditor)
				worldEditor.reset();
			else
				worldEditor.emplace(window, registry, componentRegistry);
		});

	// On envoie notre nom au serveur
	{
		PlayerNamePacket namePacket;
		namePacket.name = name;

		enet_peer_send(gameData.serverPeer, 0, build_packet(namePacket, ENET_PACKET_FLAG_RELIABLE));

		gameData.name = name;
	}

	//Inputs
	#pragma region Inputs
		Sel::InputManager::Instance().BindKeyPressed(SDLK_q, "MoveLeft");
		Sel::InputManager::Instance().BindAction("MoveLeft", [&](bool pressed)
		{
			if (gameData.playerMode == PlayerMode::Playing)
			{
				gameData.inputs.moveLeft = pressed;
			}
			else if (gameData.playerMode == PlayerMode::Dead || gameData.playerMode == PlayerMode::Spectating)
			{
				if (pressed)
				{
					// Decrement spectateIndex and wrap it using modulo
					if (gameData.spectatablePlayers.size() > 0)
					{
						gameData.previousSpectateIndex = gameData.spectateIndex;
						gameData.spectateIndex = (gameData.spectateIndex + gameData.spectatablePlayers.size() - 1) % gameData.spectatablePlayers.size();
					}
				}
			}

			//std::cout << "move Left : " << (int)(gameData.inputs.moveLeft) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_d, "MoveRight");
		Sel::InputManager::Instance().BindAction("MoveRight", [&](bool pressed)
		{
			if (gameData.playerMode == PlayerMode::Playing)
			{
				gameData.inputs.moveRight = pressed;
			}
			else if (gameData.playerMode == PlayerMode::Dead || gameData.playerMode == PlayerMode::Spectating)
			{
				if (pressed)
				{
					// Increment spectateIndex and wrap it using modulo
					if (gameData.spectatablePlayers.size() > 0)
					{
						gameData.previousSpectateIndex = gameData.spectateIndex;
						gameData.spectateIndex = (gameData.spectateIndex + 1) % gameData.spectatablePlayers.size();
					}
				}
			}

			//std::cout << "move Left : " << (int)(gameData.inputs.moveRight) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_z, "MoveUp");
		Sel::InputManager::Instance().BindAction("MoveUp", [&](bool pressed)
		{
			if (gameData.playerMode == PlayerMode::Playing)
			{
				gameData.inputs.moveUp = pressed;
			}

			//std::cout << "move Left : " << (int)(gameData.inputs.moveUp) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_s, "MoveDown");
		Sel::InputManager::Instance().BindAction("MoveDown", [&](bool pressed)
		{
			if (gameData.playerMode == PlayerMode::Playing)
			{
				gameData.inputs.moveDown = pressed;
			}

			//std::cout << "move Left : " << (int)(gameData.inputs.moveDown) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_SPACE, "Ready");
		Sel::InputManager::Instance().BindAction("Ready", [&](bool pressed)
			{
				if ((gameData.playerMode == PlayerMode::Playing || gameData.playerMode == PlayerMode::Dead) && (gameData.gameState == GameState::Lobby || gameData.gameState == GameState::EndScreen))
				{
					if (!pressed)
						return;

					gameData.isReady = !gameData.isReady;

					PlayerReadyPacket packet;
					packet.newReadyValue = gameData.isReady;
					

					enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
				}

				if (gameData.gameState == GameState::GameRunning && gameData.playerMode == PlayerMode::Playing && gameData.timeSinceLastSteal.GetElapsedTime() >= gameData.stealCooldown)
				{
					if (!pressed)
						return;

					gameData.timeSinceLastSteal.Restart();

					PlayerStealPacketRequest packet;
					packet.brawlerId = gameData.ownBrawlerNetworkIndex.value();

					enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
				}
			});
	#pragma endregion

	entt::handle cameraEntity = CreateCamera(registry);
	entt::handle cameraEntityUI = CreateCamera(registryUI);

	// On attend d'être initialisé (gameState et playerMode)
	do 
	{
		run_network(host, gameData);
	} while (gameData.playerMode == PlayerMode::Pending);

	// Le client souhaite construire son brawler
	// Il le fait s'il est en mode playing, sinon il spectate et créera son brawler à la prochaine lobby phase
	if (gameData.playerMode	== PlayerMode::Playing)
	{
		CreateBrawlerResquest packet;
		enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
		std::cout << "PLAYING MODE ACTIVATED" << std::endl;
	}
	else
	{
		std::cout << "SPECTATE MODE ACTIVATED" << std::endl;
	}

	Sel::Stopwatch clock;
	bool isOpen = true;
	while (isOpen)
	{
		float now = gameData.clock.GetElapsedTime();
		float deltaTime = clock.Restart();

		SDL_Event event;
		while (core.PollEvent(event))
		{
			if (event.type == SDL_QUIT)
				isOpen = false;

			imgui.ProcessEvent(event);

			Sel::InputManager::Instance().HandleEvent(event);
		}

		// On gère la couche réseau
		if (!run_network(host, gameData))
		{
			isOpen = false;
			break;
		}

		imgui.NewFrame();

		renderer.SetDrawColor(127, 0, 127, 255);
		renderer.Clear();

		// =============== UI LOBBY ===============
		auto uiGetReadyTextView = gameData.registryUI->view<UI_GetReadyText>();
		auto uiIsReadyTextView = gameData.registryUI->view<UI_IsReadyText>();
		if (gameData.gameState == GameState::Lobby && gameData.playerMode != PlayerMode::Dead && gameData.playerMode != PlayerMode::Spectating)
		{
			if (!gameData.isReady)
			{
				for (auto& entity : uiIsReadyTextView)
					gameData.registryUI->destroy(entity);

				if (uiGetReadyTextView.size() <= 0)
				{
					auto handle = CreateDisplayText(gameData, *(gameData.renderer), "Press SPACE to get ready", 34, Sel::Color::White, "assets/fonts/Happy Selfie.otf");
					handle.emplace<UI_GetReadyText>();
					floatingEntitySystemUI.AddFloatingEntity(cameraEntityUI, handle.entity(), { WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT - 50.f });
				}
			}
			else
			{
				for (auto& entity : uiGetReadyTextView)
					gameData.registryUI->destroy(entity);

				if (uiIsReadyTextView.size() <= 0)
				{
					auto handle = CreateDisplayText(gameData, *(gameData.renderer), "You are ready! Waiting others...", 34, Sel::Color::FromRGBA8(0, 255, 0, 255), "assets/fonts/Happy Selfie.otf");
					handle.emplace<UI_IsReadyText>();
					floatingEntitySystemUI.AddFloatingEntity(cameraEntityUI, handle.entity(), { WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT - 50.f });
				}
			}
		}
		else
		{
			if (uiIsReadyTextView.size() > 0)
			{
				for (auto& entity : uiIsReadyTextView)
					gameData.registryUI->destroy(entity);
			}
			if (uiGetReadyTextView.size() > 0)
			{
				for (auto& entity : uiGetReadyTextView)
						gameData.registryUI->destroy(entity);
			}
		}
		// =============== END UI LOBBY ===============



		// =============== UI SPECTATE MODE ===============
		auto uiSpectatingTextView = gameData.registryUI->view<UI_SpectatingText>();
		if (((gameData.playerMode == PlayerMode::Dead && gameData.gameState == GameState::GameRunning) || gameData.playerMode == PlayerMode::Spectating) && gameData.previousSpectateIndex != gameData.spectateIndex)
		{
			if (uiSpectatingTextView.size() > 0)
			{
				for (auto& entity : uiSpectatingTextView)
					gameData.registryUI->destroy(entity);
			}

			std::string spectatedName = "someone";
			auto spectateIt = std::next(gameData.spectatablePlayers.begin(), gameData.spectateIndex);
			if (spectateIt != gameData.spectatablePlayers.end())
			{
				spectatedName = spectateIt->second.name;
			}

			auto handle = CreateDisplayText(gameData, *(gameData.renderer), "<<(Q)<< Specating " + spectatedName + " >>(D)>>", 34, Sel::Color::White, "assets/fonts/Happy Selfie.otf");
			handle.emplace<UI_SpectatingText>();
			floatingEntitySystemUI.AddFloatingEntity(cameraEntityUI, handle.entity(), { WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT - 50.f });
		}
		else
		{
			if (uiSpectatingTextView.size() > 0)
			{
				for (auto& entity : uiSpectatingTextView)
					gameData.registryUI->destroy(entity);
			}
		}
		// =============== END UI SPECTATE MODE ===============
		

		// =============== UI IN GAME RUNNING ===============
		auto uiKillTimerText = gameData.registryUI->view<UI_NextKillTimerText>();
		auto uiKillTimerValue = gameData.registryUI->view<UI_NextKillTimerValue>();
		if (gameData.gameState == GameState::GameRunning)
		{
			gameData.nextKillTimer -= deltaTime;
			if (gameData.nextKillTimer <= 0)
				gameData.nextKillTimer = 10.f;

			if (uiKillTimerText.size() <= 0)
			{
				auto handle = CreateDisplayText(gameData, *(gameData.renderer), "Next kill in:", 20, Sel::Color::White, "assets/fonts/Happy Selfie.otf");
				handle.emplace<UI_NextKillTimerText>();
				floatingEntitySystemUI.AddFloatingEntity(cameraEntityUI, handle.entity(), { WINDOW_WIDTH * 0.9f, WINDOW_HEIGHT * 0.25f });
			}

			if (uiKillTimerValue.size() > 0)
			{
				for (auto& entity : uiKillTimerValue)
					gameData.registryUI->destroy(entity);
			}

			Sel::Color textColor = Sel::Color::White;
			if (gameData.nextKillTimer <= 6.0f)
				textColor = Sel::Color::Red;

			int killTimerInt = static_cast<int>(gameData.nextKillTimer);
			std::string killTimerStr = (gameData.nextKillTimer < 1.f) ? "0" : std::to_string(killTimerInt);

			auto handle = CreateDisplayText(gameData, *(gameData.renderer), killTimerStr, 30, textColor, "assets/fonts/Hey Comic.otf");
			handle.emplace<UI_NextKillTimerValue>();
			floatingEntitySystemUI.AddFloatingEntity(cameraEntityUI, handle.entity(), { WINDOW_WIDTH * 0.9f, WINDOW_HEIGHT * 0.25f + 20.f });
		}
		else
		{
			if (uiKillTimerText.size() > 0)
			{
				for (auto& entity : uiKillTimerText)
					gameData.registryUI->destroy(entity);
			}

			if (uiKillTimerValue.size() > 0)
			{
				for (auto& entity : uiKillTimerValue)
					gameData.registryUI->destroy(entity);
			}
		}
		// =============== END UI IN GAME RUNNING ===============

		// =============== UI END SCREEN ===============
		auto uiSEndScreenTextView = gameData.registryUI->view<UI_EndScreenText>();
		if (gameData.gameState == GameState::EndScreen && uiSEndScreenTextView.size() <= 0)
		{
			auto handle = CreateDisplayText(gameData, *(gameData.renderer), "Press SPACE to return to lobby", 34, Sel::Color::White, "assets/fonts/Happy Selfie.otf");
			handle.emplace<UI_SpectatingText>();
			floatingEntitySystemUI.AddFloatingEntity(cameraEntityUI, handle.entity(), { WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT - 50.f });
		}
		else
		{
			if (uiSEndScreenTextView.size() > 0)
			{
				for (auto& entity : uiSEndScreenTextView)
					gameData.registryUI->destroy(entity);
			}
		}
		// =============== END END SCREEN ===============

		auto viewBrawler = gameData.registry->view<BrawlerFlag, Sel::VelocityComponent, Sel::Transform, Sel::SpritesheetComponent>();
		for (auto&& [entity, flag, velocity, transform, spritesheetComp] : viewBrawler.each())
		{
			if (velocity.linearVel.x < 0.f)
			{
				transform.SetScale({ -1.f, 1.f });
			}
			else if (velocity.linearVel.x > 0.f)
			{
				transform.SetScale({ 1.f, 1.f });
			}

			if (gameData.registry->any_of<OneShotAnimation>(entity))
				continue;
			
			if (velocity.linearVel.Magnitude() > 0.f)
			{
				spritesheetComp.PlayAnimation("running");
			}
			else
			{
				spritesheetComp.PlayAnimation("idle");
			}
		}

		OneShotAnimationSystem(gameData, deltaTime);

		floatingEntitySystem.Update();
		floatingEntitySystemUI.Update();
		temporaryEntitySystem.Update(deltaTime);
		temporaryEntitySystemUI.Update(deltaTime);
		animationSystem.Update(deltaTime);
		renderSystem.Update(deltaTime);
		renderSystemUI.Update(deltaTime);

		if (ImGui::Begin("Menu"))
		{
			ImGui::LabelText("FPS", "%f", 1.f / deltaTime);

			ImGui::Text("Nombre d'entités: %zu", registry.storage<entt::entity>().in_use());

			/*if (ImGui::CollapsingHeader("Connectés"))
			{
				for (const auto& playerData : gameData.players)
					ImGui::TextColored({ playerData.color.r, playerData.color.g, playerData.color.b, playerData.color.a }, "%s", playerData.name.c_str());
			}*/
		}
		ImGui::End();

		

		// Center camera on our brawler if not spectating
		if (gameData.ownBrawlerNetworkIndex && gameData.playerMode == PlayerMode::Playing)
		{
			auto it = gameData.networkToEntities.find(gameData.ownBrawlerNetworkIndex.value());
			if (it != gameData.networkToEntities.end())
			{
				auto& transformCamera = registry.get<Sel::Transform>(cameraEntity);
				auto& transformEntity = registry.get<Sel::Transform>(it->second);
				transformCamera.SetPosition(transformEntity.GetGlobalPosition() - Sel::Vector2f(WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f));
			}
		}
		else if(gameData.spectatablePlayers.size() > 0)
		{
			if (gameData.beforeSpectateClock.GetElapsedTime() >= gameData.waitBeforeSpectate)
			{
				// Clamp spectateIndex to the size of spectatablePlayers
				if (gameData.spectateIndex >= gameData.spectatablePlayers.size())
				{
					gameData.spectateIndex = gameData.spectatablePlayers.size() - 1;
				}

				// Find the player in spectablePlayers
				auto spectateIt = std::next(gameData.spectatablePlayers.begin(), gameData.spectateIndex);
				if (spectateIt != gameData.spectatablePlayers.end())
				{
					const PlayerData& playerData = spectateIt->second;

					// If the player has an ownBrawlerId, find the corresponding entity
					if (playerData.ownBrawlerId.has_value())
					{
						auto entityIt = gameData.networkToEntities.find(playerData.ownBrawlerId.value());
						if (entityIt != gameData.networkToEntities.end())
						{
							// Get the transform of the entity and make the camera follow it
							auto& transformCamera = registry.get<Sel::Transform>(cameraEntity);
							auto& transformEntity = registry.get<Sel::Transform>(entityIt->second);
							transformCamera.SetPosition(transformEntity.GetGlobalPosition() - Sel::Vector2f(WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f));
						}
					}
				}
			}
		}

		// Align ui camera with game one;
		cameraEntityUI.get<Sel::Transform>().SetPosition(cameraEntity.get<Sel::Transform>().GetGlobalPosition());

		if (worldEditor)
			worldEditor->Render();

		imgui.Render(renderer);

		renderer.Present();

		// On vérifie si assez de temps s'est écoulé pour faire avancer la logique du jeu
		if (now >= gameData.nextTick)
		{
			//worldLimit.Update();

			// On met à jour la logique du jeu
			tick(gameData);

			// On prévoit la prochaine mise à jour
			gameData.nextTick += gameData.tickInterval;
		}
	}

	// On prévient le serveur qu'on s'est déconnecté
	enet_peer_disconnect_now(gameData.serverPeer, 0);
	enet_host_flush(host);
	enet_host_destroy(host);

	enet_deinitialize();

	return EXIT_SUCCESS;
}

entt::handle CreateCamera(entt::registry& registry)
{
	entt::entity entity = registry.create();
	registry.emplace<Sel::CameraComponent>(entity);
	registry.emplace<Sel::Transform>(entity);

	return entt::handle(registry, entity);
}


Sel::Sprite BuildCollectibleSprite(float size, const CollectibleType& type)
{
	Sel::ResourceManager& resourceManager = Sel::ResourceManager::Instance();
	switch (type)
	{
		case CollectibleType::Carrot:
		{
			Sel::Sprite collectibleSprite(resourceManager.GetTexture("assets/ball.png"));

			collectibleSprite.Resize(size, size);
			collectibleSprite.SetOrigin({ 0.5f, 0.5f });
			collectibleSprite.SetColor(Sel::Color(128.f, 0, 0));

			return collectibleSprite;
		}
		case CollectibleType::GoldenCarrot:
		{
			Sel::Sprite collectibleSprite(resourceManager.GetTexture("assets/ball.png"));

			collectibleSprite.Resize(size * 2, size * 2);
			collectibleSprite.SetOrigin({ 0.5f, 0.5f });
			collectibleSprite.SetColor(Sel::Color(0, 128.f, 0));

			return collectibleSprite;
		}
	}
}

entt::handle SpawnCollectible(GameData& gameData, const CreateCollectiblePacket& packet)
{
	entt::entity newCollectible = gameData.registry->create();

	// Add Transform
	auto& transform = gameData.registry->emplace<Sel::Transform>(newCollectible);
	transform.SetPosition(packet.position);
	transform.SetRotation(0.f);
	transform.SetScale({ packet.scale, packet.scale });

	auto& collectibleType = gameData.registry->emplace<CollectibleFlag>(newCollectible);
	collectibleType.type = packet.type;

	gameData.registry->emplace<GoldenCarrotFlag>(newCollectible);

	// Add graphics component
	auto& gfxComponent = gameData.registry->emplace<Sel::GraphicsComponent>(newCollectible);
	gfxComponent.renderable = std::make_shared<Sel::Sprite>(BuildCollectibleSprite(75.f, packet.type));

	// On crée un handle
	entt::handle handle = entt::handle(*(gameData.registry), newCollectible);

	// On l'ajoute à la liste d'entité
	gameData.networkToEntities[packet.collectibleId] = handle;

	return handle;
}

entt::handle CreateDisplayText(GameData& gameData, Sel::Renderer& renderer, std::string text, int fontSize, const Sel::Color& textColor, const std::string& fontPath, Sel::Vector2f origin, bool isUI)
{
	std::shared_ptr<Sel::Font> font = Sel::ResourceManager::Instance().GetFont(fontPath);
	Sel::Surface surface = font->RenderUTF8Text(fontSize, text);
	std::shared_ptr<Sel::Texture> tex = std::make_shared<Sel::Texture>(Sel::Texture::CreateFromSurface(renderer, surface));
	std::shared_ptr<Sel::Sprite> sprite = std::make_shared<Sel::Sprite>(tex);
	sprite->SetColor(textColor);
	sprite->SetOrigin(origin);

	entt::entity entityText;
	entt::handle handle;
	if (!isUI)
	{
		entityText = gameData.registry->create();
		gameData.registry->emplace<Sel::Transform>(entityText);
		gameData.registry->emplace<Sel::GraphicsComponent>(entityText).renderable = std::move(sprite);

		handle = entt::handle(*(gameData.registry), entityText);
	}
	else
	{
		entityText = gameData.registryUI->create();
		gameData.registryUI->emplace<Sel::Transform>(entityText);
		gameData.registryUI->emplace<Sel::GraphicsComponent>(entityText).renderable = std::move(sprite);

		handle = entt::handle(*(gameData.registryUI), entityText);
	}

	return handle;
}

void OneShotAnimationSystem(GameData& gameData, float deltaTime)
{
	auto view = gameData.registry->view<OneShotAnimation, Sel::SpritesheetComponent>();
	for (auto&& [entity, animation, spritesheetComp] : view.each())
	{
		if (!animation.isPlaying)
		{
			spritesheetComp.PlayAnimation(animation.animationName);
			animation.isPlaying = true;
		}

		animation.animationDuration -= deltaTime;

		if (animation.animationDuration <= 0.f)
			gameData.registry->remove<OneShotAnimation>(entity);
	}
}

void handle_message(const std::vector<std::uint8_t>& message, GameData& gameData)
{
	// On décode l'opcode pour savoir à quel type de message on a affaire
	std::size_t offset = 0;
	Opcode opcode = static_cast<Opcode>(Deserialize_u8(message, offset));
	switch (opcode)
	{
		case Opcode::S_PlayerList:
		{
			PlayerListPacket packet = PlayerListPacket::Deserialize(message, offset);

			// Create a temporary map for the new player list from the packet
			std::map<std::uint32_t, PlayerData> newPlayers;

			for (const auto& packetPlayer : packet.players)
			{
				PlayerData player;
				player.name = packetPlayer.name;
				player.isDead = packetPlayer.isDead;
				if (packetPlayer.hasBrawler)
					player.ownBrawlerId = packetPlayer.brawlerId;

				newPlayers[packetPlayer.id] = player;
			}

			// Compare and track disconnected players
			for (const auto& [playerId, playerData] : gameData.players)
			{
				if (newPlayers.find(playerId) == newPlayers.end())
				{
					// Player is not in the new list (disconnected), remove from spectatablePlayers
					gameData.spectatablePlayers.erase(playerId);
					gameData.previousSpectateIndex = 50; // random number above player limits to trigger UI change
				}
			}

			gameData.players = std::move(newPlayers);

			break;
		}

		case Opcode::S_CreateBrawler:
		{
			CreateBrawlerPacket packet = CreateBrawlerPacket::Deserialize(message, offset);
			Sel::Vector2f position = packet.position;
			Sel::Vector2f linearVelocity = packet.linearVelocity;
			float scale = packet.scale;

			std::string brawlerName;
			auto playerDataIt = gameData.players.find(packet.playerId);
			if (playerDataIt != gameData.players.end())
			{
				playerDataIt->second.ownBrawlerId = packet.brawlerId;
				brawlerName = playerDataIt->second.name;
			}

			if (brawlerName.empty())
				brawlerName = "Default Name";

			

			BrawlerClient brawler(*(gameData.registry), position, 0.f, scale, linearVelocity);
			auto brawlerNameEntity = CreateDisplayText(gameData, *(gameData.renderer), brawlerName, 26, Sel::Color::White, "assets/fonts/Hey Comic.otf", {0.5f, 0.5f}, false);

			gameData.floatingEntitySystem->AddFloatingEntity(brawler.GetHandle().entity(), brawlerNameEntity.entity(), { 0.f, -40.f });

			gameData.networkToEntities[packet.brawlerId] = brawler.GetHandle();


			/*std::cout << "new Brawler" << std::endl;*/

			break;
		}
		
		case Opcode::S_CreateCollectible:
		{
			CreateCollectiblePacket packet = CreateCollectiblePacket::Deserialize(message, offset);

			SpawnCollectible(gameData, packet);

			if(packet.type == CollectibleType::GoldenCarrot)
				std::cout << "new GoldenCarrot" << std::endl;
			else
				std::cout << "new Collectible" << std::endl;


			break;
		}

		case Opcode::S_DeleteBrawler:
		{
			DeleteEntityPacket packet = DeleteEntityPacket::Deserialize(message, offset);

			auto it = gameData.networkToEntities.find(packet.brawlerId);
			if (it == gameData.networkToEntities.end())
				break;

			gameData.registry->destroy(it->second);

			gameData.networkToEntities.erase(it);

			if (gameData.ownBrawlerNetworkIndex && packet.brawlerId == gameData.ownBrawlerNetworkIndex)
				gameData.ownBrawlerNetworkIndex.reset();
			
			break;
		}

		case Opcode::S_BrawlerStates:
		{
			BrawlerStatesPacket packet = BrawlerStatesPacket::Deserialize(message, offset);


			for (const auto& state : packet.brawlers)
			{
				auto it = gameData.networkToEntities.find(state.brawlerId);
				if (it == gameData.networkToEntities.end())
					continue;

				entt::handle brawlerEntity = it->second;

				auto& transform = brawlerEntity.get<Sel::Transform>();
				transform.SetPosition(state.position);
				
				auto velocity = brawlerEntity.try_get<Sel::VelocityComponent>();
				if(velocity)
					velocity->linearVel = state.linearVelocity;
			}

			gameData.snapshots.push_back(std::move(packet));

			break;
		}

		case Opcode::S_UpdateSelfBrawlerId:
		{
			UpdateSelfBrawlerId packet = UpdateSelfBrawlerId::Deserialize(message, offset);
			
			gameData.ownBrawlerNetworkIndex = packet.id;

			gameData.playerMode = PlayerMode::Playing;

			break;
		}

		case Opcode::S_CollectibleCollected:
		{
			std::cout << "Collectible Recupere" << std::endl;

			break;
		}

		case Opcode::S_UpdateGameState:
		{
			UpdateGameStatePacket packet = UpdateGameStatePacket::Deserialize(message, offset);

			switch (static_cast<GameState>(packet.newGameState))
			{
				case GameState::Lobby:
				{
					gameData.isReady = false;
					// j'étais spectateur, je peux mtnt jouer je crée mon brawler
					if (gameData.playerMode == PlayerMode::Spectating)
					{
						CreateBrawlerResquest packet;
						enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
						gameData.playerMode = PlayerMode::Playing;
					}
					else if (gameData.playerMode == PlayerMode::Dead)
					{
						gameData.playerMode = PlayerMode::Playing;
					}
					else
					{
						gameData.playerScore = 0;
					}

					// Clear UI
					if (gameData.playerMode != PlayerMode::Pending)
					{
						// Delete all entities with LeaderBoardLine
						auto leaderboardLineView = gameData.registryUI->view<LeaderBoardLine>();
						for (auto entity : leaderboardLineView) {
							gameData.registryUI->destroy(entity); // Delete entities with LeaderBoardLine flag
						}
					}

					break;
				}
				case GameState::GameRunning:
				{
					if (gameData.playerMode == PlayerMode::Playing)
					{
						gameData.spectatablePlayers.clear();

						// La game se lance, je met tous les joueurs présents à vivant
						for (auto& player : gameData.players)
							player.second.isDead = false;

						// je les ajoute à la liste des spectables
						gameData.spectatablePlayers = gameData.players;

						break;
					}

					break;
				}
			}

			std::cout << "GameState: " << (int)(gameData.gameState) << " -> " << (int)(packet.newGameState) << std::endl;

			gameData.gameState = static_cast<GameState>(packet.newGameState);
			break;
		}

		case Opcode::S_UpdatePlayerMode:
		{
			UpdatePlayerModePacket packet = UpdatePlayerModePacket::Deserialize(message, offset);

			gameData.playerMode = static_cast<PlayerMode>(packet.newPlayerMode);

			if (gameData.playerMode == PlayerMode::Spectating || gameData.playerMode == PlayerMode::Dead)
			{
				for (auto& player : gameData.players)
				{
					if (!player.second.ownBrawlerId.has_value())
						player.second.isDead = true;

					if (!player.second.isDead && player.second.ownBrawlerId.has_value()) // Je recupère donc les joueurs pas morts et qui ont un brawler
						gameData.spectatablePlayers[player.first] = player.second;
				}
				break;
			}

			break;
		}

		case Opcode::S_UpdateLeaderboard:
		{
			// get camera
			auto view = gameData.registryUI->view<Sel::CameraComponent, Sel::Transform>();
			entt::entity cameraEntityUI;
			if (view.begin() != view.end())
				cameraEntityUI = view.front();
			else
				break; // no camera to anchor the leaderboard

			// Delete all entities with LeaderBoardLine
			auto leaderboardLineView = gameData.registryUI->view<LeaderBoardLine>();
			for (auto entity : leaderboardLineView) {
				gameData.registryUI->destroy(entity); // Delete entities with LeaderBoardLine flag
			}

			int fontSize = 24;
			UpdateLeaderboardPacket packet = UpdateLeaderboardPacket::Deserialize(message, offset);

			float displayOffset = 0.f;
			float interlineOffset = 5.f;
			int index = 0;
			for (auto& onePlayer : packet.leaderboard)
			{
				std::string textToDisplay = onePlayer.playerName + " | " + std::to_string(onePlayer.playerScore);
				Sel::Color textColor = onePlayer.isDead ? Sel::Color::Red : Sel::Color::White;
				if (!onePlayer.isDead)
				{
					if (index < packet.leaderboard.size() - 1)
						textColor = packet.leaderboard[index + 1].isDead ? Sel::Color::FromRGBA8(255, 165, 0, 255) : Sel::Color::White; // Je suis pas mort mais en danger si le suivant est mort | safe si le suivant est vivant
					else
						textColor = Sel::Color::FromRGBA8(255, 165, 0, 255); // Je suis le dernier du leaderboard et je suis encore en vie = danger
				}

				auto textEntityHandle = CreateDisplayText(gameData, *(gameData.renderer), textToDisplay, fontSize, textColor, "assets/fonts/Happy Selfie.otf", {0.f, 0.f}, true);
				gameData.registryUI->emplace<LeaderBoardLine>(textEntityHandle);

				gameData.floatingEntitySystemUI->AddFloatingEntity(cameraEntityUI, textEntityHandle, { 0.f + SCREEN_MARGIN, displayOffset + SCREEN_MARGIN });

				displayOffset += (float)fontSize + interlineOffset;
				index++;
			}

			break;
		}

		case Opcode::S_BrawlerDeath:
		{
			BrawlerDeathPacket packet = BrawlerDeathPacket::Deserialize(message, offset);

			if (packet.brawlerId == gameData.ownBrawlerNetworkIndex)
			{
				std::cout << "Im Dead" << std::endl;
				gameData.playerMode = PlayerMode::Dead;
				gameData.beforeSpectateClock.Restart();
			}

			bool bFlip = true;
			auto brawlerIt = gameData.networkToEntities.find(packet.brawlerId);
			if (brawlerIt != gameData.networkToEntities.end())
			{
				bFlip = brawlerIt->second.try_get<Sel::Transform>()->GetScale().x > 0 ? false : true;
			}

			// Spawn temp entity for death anim
			BrawlerClient::BuildTemp(*(gameData.registry), packet.deathPosition, bFlip);

			// Mark the player as dead in gameData.players
			auto it = gameData.players.find(packet.playerId);
			if (it != gameData.players.end())
				it->second.isDead = true;

			// Remove the player from gameData.spectablePlayers if it exists
			auto spectableIt = gameData.spectatablePlayers.find(packet.playerId);
			if (spectableIt != gameData.spectatablePlayers.end())
				gameData.spectatablePlayers.erase(spectableIt);

			break;
		}
		
		case Opcode::S_PlayerSteal:
		{
			PlayerStealPacket packet = PlayerStealPacket::Deserialize(message, offset);

			auto it = gameData.networkToEntities.find(packet.brawlerId); 
			if (it != gameData.networkToEntities.end())
			{
				it->second.emplace_or_replace<OneShotAnimation>(false, "steal", 0.3f);
			}

			break;
		}

		case Opcode::S_Winner:
		{
			WinnerPacket packet = WinnerPacket::Deserialize(message, offset);

			if (!gameData.ownBrawlerNetworkIndex)
				break;

			if (gameData.ownBrawlerNetworkIndex == packet.brawlerNetworkId)
			{
				std::cout << "I am the winner" << std::endl;
			}

			break;
		}
	}
}

bool run_network(ENetHost* host, GameData& gameData)
{
	ENetEvent event;
	// On fait tourner le réseau via enet_host_service
	if (enet_host_service(host, &event, 0) > 0)
	{
		// On dépile tous les événements
		// enet_host_check_events permet de récupérer les événements sans faire tourner la couche réseau d'ENet (la meilleure approche est donc d'appeler enet_host_service une fois puis enet_host_check_events tant que celle-ci renvoie une valeur supérieure à zéro)

		do
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "Disconnected from server" << std::endl;
				return false;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				// On a reçu un message ! Traitons-le
				std::vector<std::uint8_t> content(event.packet->dataLength); //< On copie son contenu dans un std::vector pour plus de facilité de gestion
				std::memcpy(content.data(), event.packet->data, event.packet->dataLength);

				// On gère le message qu'on a reçu
				handle_message(content, gameData);

				// On n'oublie pas de libérer le packet
				enet_packet_destroy(event.packet);
				break;
			}
			}
		} while (enet_host_check_events(host, &event) > 0);
	}

	return true;
}


void tick(GameData& gameData)
{
	PlayerInputsPacket playerInputs;
	playerInputs.brawlerId = *(gameData.ownBrawlerNetworkIndex);
	playerInputs.inputs = gameData.inputs;

	enet_peer_send(gameData.serverPeer, 0, build_packet(playerInputs, 0));
}

