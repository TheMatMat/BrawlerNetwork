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

struct PlayerData
{
	std::string name;
	std::optional<std::uint32_t> ownBrawlerId;
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
	std::uint8_t spectateIndex;
	std::uint8_t playerScore; 

	std::string name;
	std::map<std::uint32_t, PlayerData> players;
	ENetPeer* serverPeer; //< Le serveur
	entt::registry* registry;
	Sel::Renderer* renderer;
	FloatingEntitySystem* floatingEntitySystem;
	std::unordered_map<std::uint32_t /*networkId*/, entt::handle> networkToEntities; //< toutes les entit�s (ici tous les brawlers)
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
Sel::Sprite BuildCollectibleSprite(float size);
entt::handle SpawnCollectible(GameData& gameData, const CreateCollectiblePacket& packet);
entt::handle CreateDisplayText(GameData& gameData, Sel::Renderer& renderer, std::string text, int fontSize, const Sel::Color& textColor, const std::string& fontPath, Sel::Vector2f origin = {0.5f, 0.5f});

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

		// On ne cr�� l'host qu'une fois qu'on connait le type d'adresse (IPv4/IPv6)
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
			// Note : il est important d'appeler plusieurs fois la fonction plut�t qu'une seule fois dans ce cas, pour autoriser plusieurs tentatives de connexion
			// (l'API ENet ayant �t� con�ue autour du fait qu'enet_host_service �tait appel� en boucle)

			// Nous simulons �a avec une boucle, o� chaque tour de boucle va attendre 100ms (pour un total de 5s)
			for (std::size_t i = 0; i < 50; ++i)
			{
				ENetEvent event;
				if (enet_host_service(host, &event, 100) > 0)
				{
					// Nous avons un �v�nement, la connexion a soit pu s'effectuer (ENET_EVENT_TYPE_CONNECT) soit �chou� (ENET_EVENT_TYPE_DISCONNECT)
					break; //< On sort de la boucle
				}
			}

			// On v�rifie apr�s la boucle l'�tat du peer, sommes-nous connect�s ?
			if (gameData.serverPeer->state != ENET_PEER_STATE_CONNECTED)
			{
				// On force la r�initialisation du serveur aupr�s d'enet pour pouvoir allouer un nouveau peer
				enet_peer_reset(gameData.serverPeer);

				std::cout << "failed to connect." << std::endl;
				continue;
			}

			// Si on arrive ici, on est connect� !
			std::cout << "Connected!" << std::endl;
			break; //< On casse la boucle demandant une IP pour se connecter
		}

		break;
	}



	Sel::Core core;

	Sel::Window window("Braaaaaawl", WINDOW_WIDTH, WINDOW_LENGHT);
	Sel::Renderer renderer(window, SDL_RENDERER_PRESENTVSYNC);
	gameData.renderer = &renderer;

	Sel::ResourceManager resourceManager(renderer);
	Sel::InputManager inputManager;

	Sel::ImGuiRenderer imgui(window, renderer);
	ImGui::SetCurrentContext(imgui.GetContext());

	entt::registry registry;

	/// ============ Animation & Rendering ============
	Sel::RenderSystem renderSystem(renderer, registry);

	Sel::AnimationSystem animationSystem(registry);
	

	Sel::PhysicsSystem physicsSystem(registry);
	physicsSystem.SetGravity({ 0.f, 0.f });
	physicsSystem.SetDamping(0.9f);

	Sel::VelocitySystem velocitySystem(registry);

	FloatingEntitySystem floatingEntitySystem(&registry);
	gameData.floatingEntitySystem = &floatingEntitySystem;

	gameData.registry = &registry;
	gameData.gameState = GameState::Lobby;

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
			if (!pressed)
				gameData.inputs.moveLeft = false;
			else
				gameData.inputs.moveLeft = true;

			//std::cout << "move Left : " << (int)(gameData.inputs.moveLeft) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_d, "MoveRight");
		Sel::InputManager::Instance().BindAction("MoveRight", [&](bool pressed)
		{
			if (!pressed)
				gameData.inputs.moveRight = false;
			else
				gameData.inputs.moveRight = true;

			//std::cout << "move Left : " << (int)(gameData.inputs.moveRight) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_z, "MoveUp");
		Sel::InputManager::Instance().BindAction("MoveUp", [&](bool pressed)
		{
			if (!pressed)
				gameData.inputs.moveUp = false;
			else
				gameData.inputs.moveUp = true;

			//std::cout << "move Left : " << (int)(gameData.inputs.moveUp) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_s, "MoveDown");
		Sel::InputManager::Instance().BindAction("MoveDown", [&](bool pressed)
		{
			if (!pressed)
				gameData.inputs.moveDown = false;
			else
				gameData.inputs.moveDown = true;

			//std::cout << "move Left : " << (int)(gameData.inputs.moveDown) << std::endl;
		});

		Sel::InputManager::Instance().BindKeyPressed(SDLK_SPACE, "Ready");
		Sel::InputManager::Instance().BindAction("Ready", [&](bool pressed)
			{
				if (gameData.gameState == GameState::Lobby || gameData.gameState == GameState::EndScreen)
				{
					if (!pressed)
						return;

					gameData.isReady = !gameData.isReady;

					PlayerReadyPacket packet;
					packet.newReadyValue = gameData.isReady;
					

					enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
				}
			});
	#pragma endregion

	entt::handle cameraEntity = CreateCamera(registry);

	// Le client souhaite construire son brawler
	if (gameData.gameState == GameState::Lobby)
	{
		CreateBrawlerResquest packet;
		enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
	}
	else
	{
		gameData.playerMode = PlayerMode::Spectating;
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

		// On g�re la couche r�seau
		if (!run_network(host, gameData))
		{
			isOpen = false;
			break;
		}

		imgui.NewFrame();

		renderer.SetDrawColor(127, 0, 127, 255);
		renderer.Clear();

		/*brawler.ApplyInputs(gameData.inputs);*/

		//physicsSystem.Update(deltaTime);
		//velocitySystem.Update(deltaTime);

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
			
			if (velocity.linearVel.Magnitude() > 0.f)
			{
				spritesheetComp.PlayAnimation("running");
			}
			else
			{
				spritesheetComp.PlayAnimation("idle");
			}
		}

		floatingEntitySystem.Update();
		renderSystem.Update(deltaTime);
		animationSystem.Update(deltaTime);

		if (ImGui::Begin("Menu"))
		{
			ImGui::LabelText("FPS", "%f", 1.f / deltaTime);

			ImGui::Text("Nombre d'entit�s: %zu", registry.storage<entt::entity>().in_use());

			/*if (ImGui::CollapsingHeader("Connect�s"))
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
				transformCamera.SetPosition(transformEntity.GetGlobalPosition() - Sel::Vector2f(WINDOW_WIDTH * 0.5f, WINDOW_LENGHT * 0.5f));
			}
		}

		if (worldEditor)
			worldEditor->Render();

		imgui.Render(renderer);

		renderer.Present();

		// On v�rifie si assez de temps s'est �coul� pour faire avancer la logique du jeu
		if (now >= gameData.nextTick)
		{
			//worldLimit.Update();

			// On met � jour la logique du jeu
			tick(gameData);

			// On pr�voit la prochaine mise � jour
			gameData.nextTick += gameData.tickInterval;
		}
	}

	// On pr�vient le serveur qu'on s'est d�connect�
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


Sel::Sprite BuildCollectibleSprite(float size)
{
	Sel::ResourceManager& resourceManager = Sel::ResourceManager::Instance();
	Sel::Sprite collectibleSprite(resourceManager.GetTexture("assets/ball.png"));

	collectibleSprite.Resize(size, size);
	collectibleSprite.SetOrigin({ 0.5f, 0.5f });
	collectibleSprite.SetColor(Sel::Color(128.f, 0, 0));

	return collectibleSprite;
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
	collectibleType.type = CollectibleType::Fire;

	// Add graphics component
	auto& gfxComponent = gameData.registry->emplace<Sel::GraphicsComponent>(newCollectible);
	gfxComponent.renderable = std::make_shared<Sel::Sprite>(BuildCollectibleSprite(75.f));

	// On cr�e un handle
	entt::handle handle = entt::handle(*(gameData.registry), newCollectible);

	// On l'ajoute � la liste d'entit�
	gameData.networkToEntities[packet.collectibleId] = handle;

	return handle;
}

entt::handle CreateDisplayText(GameData& gameData, Sel::Renderer& renderer, std::string text, int fontSize, const Sel::Color& textColor, const std::string& fontPath, Sel::Vector2f origin)
{
	std::shared_ptr<Sel::Font> font = Sel::ResourceManager::Instance().GetFont(fontPath);
	Sel::Surface surface = font->RenderUTF8Text(fontSize, text);
	std::shared_ptr<Sel::Texture> tex = std::make_shared<Sel::Texture>(Sel::Texture::CreateFromSurface(renderer, surface));
	std::shared_ptr<Sel::Sprite> sprite = std::make_shared<Sel::Sprite>(tex);
	sprite->SetColor(textColor);
	sprite->SetOrigin(origin);

	auto entityText = gameData.registry->create();
	gameData.registry->emplace<Sel::Transform>(entityText);
	gameData.registry->emplace<Sel::GraphicsComponent>(entityText).renderable = std::move(sprite);

	// On cr�e un handle
	entt::handle handle = entt::handle(*(gameData.registry), entityText);

	return handle;
}

void handle_message(const std::vector<std::uint8_t>& message, GameData& gameData)
{
	// On d�code l'opcode pour savoir � quel type de message on a affaire
	std::size_t offset = 0;
	Opcode opcode = static_cast<Opcode>(Deserialize_u8(message, offset));
	switch (opcode)
	{
		case Opcode::S_PlayerList:
		{
			PlayerListPacket packet = PlayerListPacket::Deserialize(message, offset);

			// On reconstruit la liste des joueurs
			gameData.players.clear();
			for (const auto& packetPlayer : packet.players)
			{
				PlayerData player;
				player.name = packetPlayer.name;
				if (packetPlayer.hasBrawler)
					player.ownBrawlerId = packetPlayer.brawlerId;

				gameData.players[packetPlayer.id] = player;
			}
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
			auto brawlerNameEntity = CreateDisplayText(gameData, *(gameData.renderer), brawlerName, 26, Sel::Color::White, "assets/fonts/Hey Comic.otf");

			gameData.floatingEntitySystem->AddFloatingEntity(brawler.GetHandle().entity(), brawlerNameEntity.entity(), { 0.f, -40.f });

			gameData.networkToEntities[packet.brawlerId] = brawler.GetHandle();


			std::cout << "new Brawler" << std::endl;

			break;
		}
		
		case Opcode::S_CreateCollectible:
		{
			CreateCollectiblePacket packet = CreateCollectiblePacket::Deserialize(message, offset);

			SpawnCollectible(gameData, packet);

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

				auto& velocity = brawlerEntity.get<Sel::VelocityComponent>();
				velocity.linearVel = state.linearVelocity;
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
					// j'�tais spectateur, je peux mtnt jouer je cr�e mon brawler
					if (gameData.playerMode == PlayerMode::Spectating)
					{
						CreateBrawlerResquest packet;
						enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
					}
					else
					{
						gameData.playerScore = 0;
					}

					break;
				}
			}

			std::cout << "GameState: " << (int)(gameData.gameState) << " -> " << (int)(packet.newGameState) << std::endl;

			gameData.gameState = static_cast<GameState>(packet.newGameState);
			break;
		}

		case Opcode::S_UpdateLeaderboard:
		{
			// get camera
			auto view = gameData.registry->view<Sel::CameraComponent, Sel::Transform>();
			entt::entity cameraEntity;
			if (view.begin() != view.end())
				cameraEntity = view.front();
			else
				break; // no camera to anchor the leaderboard

			// Delete all entities with LeaderBoardLine
			auto leaderboardLineView = gameData.registry->view<LeaderBoardLine>();
			for (auto entity : leaderboardLineView) {
				gameData.registry->destroy(entity); // Delete entities with LeaderBoardLine flag
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

				auto textEntityHandle = CreateDisplayText(gameData, *(gameData.renderer), textToDisplay, fontSize, textColor, "assets/fonts/Happy Selfie.otf", {0.f, 0.f});
				gameData.registry->emplace<LeaderBoardLine>(textEntityHandle);

				gameData.floatingEntitySystem->AddFloatingEntity(cameraEntity, textEntityHandle, { 0.f + SCREEN_MARGIN, displayOffset + SCREEN_MARGIN });

				displayOffset += (float)fontSize + interlineOffset;
				index++;
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
	// On fait tourner le r�seau via enet_host_service
	if (enet_host_service(host, &event, 0) > 0)
	{
		// On d�pile tous les �v�nements
		// enet_host_check_events permet de r�cup�rer les �v�nements sans faire tourner la couche r�seau d'ENet (la meilleure approche est donc d'appeler enet_host_service une fois puis enet_host_check_events tant que celle-ci renvoie une valeur sup�rieure � z�ro)

		do
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "Disconnected from server" << std::endl;
				return false;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				// On a re�u un message ! Traitons-le
				std::vector<std::uint8_t> content(event.packet->dataLength); //< On copie son contenu dans un std::vector pour plus de facilit� de gestion
				std::memcpy(content.data(), event.packet->data, event.packet->dataLength);

				// On g�re le message qu'on a re�u
				handle_message(content, gameData);

				// On n'oublie pas de lib�rer le packet
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