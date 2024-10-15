#pragma once

#include "sh_constants.h"
#include "enet6/enet.h"
#include "entt/entt.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <vector>
#include <unordered_map>
#include <Sel/Color.hpp>
#include <Sel/AnimationSystem.hpp>
#include <Sel/CameraComponent.hpp>
#include <Sel/ComponentRegistry.hpp>
#include <Sel/Stopwatch.hpp>
#include <Sel/GraphicsComponent.hpp>
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

struct PlayerData
{
	std::string name;
	Sel::Color color;
	std::optional<BrawlerClient> brawler;
	std::optional<std::uint8_t> deathPositionIndex;
};

struct ClientNetworkId
{
	std::uint32_t networkId;
};

struct GameData
{
	std::vector<PlayerData> players;
	ENetPeer* serverPeer; //< Le serveur
	entt::registry* registry;
	std::unordered_map<std::uint32_t /*networkId*/, entt::handle> networkToEntities; //< toutes les entités (ici tous les brawlers)
	PlayerInputs inputs; //< Les inputs du joueur
	std::size_t ownPlayerIndex; //< Notre propre ID
	std::uint32_t ownBrawlerNetworkIndex; //< l'ID reseau de notre brawler
};

void handle_message(const std::vector<std::uint8_t>& message, GameData& gameData);
bool run_network(ENetHost* host, GameData& gameData);
void tick(GameData& gameData);

entt::handle CreateCamera(entt::registry& registry);

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

	Sel::Window window("Braaaaaawl", 1280, 720);
	Sel::Renderer renderer(window, SDL_RENDERER_PRESENTVSYNC);

	Sel::ResourceManager resourceManager(renderer);
	Sel::InputManager inputManager;

	Sel::ImGuiRenderer imgui(window, renderer);
	ImGui::SetCurrentContext(imgui.GetContext());

	entt::registry registry;
	Sel::RenderSystem renderSystem(renderer, registry);
	Sel::PhysicsSystem physicsSystem(registry);
	physicsSystem.SetGravity({ 0.f, 0.f });
	physicsSystem.SetDamping(0.9f);

	Sel::VelocitySystem velocitySystem(registry);

	gameData.registry = &registry;

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
	#pragma endregion

	entt::handle cameraEntity = CreateCamera(registry);

	// Le client souhaite construire son brawler
	CreateBrawlerResquest packet;
	enet_peer_send(gameData.serverPeer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));

	Sel::Stopwatch clock;
	bool isOpen = true;
	while (isOpen)
	{
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

		/*brawler.ApplyInputs(gameData.inputs);*/

		//physicsSystem.Update(deltaTime);
		//velocitySystem.Update(deltaTime);
		renderSystem.Update(deltaTime);

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

		if (worldEditor)
			worldEditor->Render();

		imgui.Render(renderer);

		renderer.Present();

		tick(gameData);
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

void handle_message(const std::vector<std::uint8_t>& message, GameData& gameData)
{
	// On décode l'opcode pour savoir à quel type de message on a affaire
	std::size_t offset = 0;
	Opcode opcode = static_cast<Opcode>(Deserialize_u8(message, offset));
	switch (opcode)
	{
		case Opcode::S_CreateBrawler:
		{
			CreateBrawlerPacket packet = CreateBrawlerPacket::Deserialize(message, offset);
			Sel::Vector2f position = packet.position;
			Sel::Vector2f linearVelocity = packet.linearVelocity;

			BrawlerClient brawler(*(gameData.registry), position, 0.f, 1.f, linearVelocity);

			gameData.networkToEntities[packet.brawlerId] = brawler.GetHandle();

			std::cout << "new Brawler" << std::endl;

			break;
		}

		case Opcode::S_DeleteBrawler:
		{
		
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
			break;
		}

		case Opcode::S_UpdateSelfBrawlerId:
		{
			UpdateSelfBrawlerId packet = UpdateSelfBrawlerId::Deserialize(message, offset);
			
			gameData.ownBrawlerNetworkIndex = packet.id;

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
	playerInputs.brawlerId = gameData.ownBrawlerNetworkIndex;
	playerInputs.inputs = gameData.inputs;

	enet_peer_send(gameData.serverPeer, 0, build_packet(playerInputs, 0));
}