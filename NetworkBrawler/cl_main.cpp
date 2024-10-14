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
#include <Sel/Core.hpp>
#include <Sel/Renderer.hpp>
#include <Sel/InputManager.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/Window.hpp>
#include <Sel/Stopwatch.hpp>
#include "cl_brawler.h"
#include "sh_inputs.h"
#include <Sel/ChipmunkSpace.hpp>

struct PlayerData
{
	std::string name;
	Sel::Color color;
	std::optional<ClientBrawler> brawler;
	std::optional<std::uint8_t> deathPositionIndex;
};

struct GameData
{
	std::vector<PlayerData> players;
	ENetPeer* serverPeer; //< Le serveur
	entt::registry* registry;
	std::unordered_map<std::uint32_t /*networkId*/, entt::handle> networkToEntities; //< tout le monde
	PlayerInputs inputs; //< Les inputs du joueur
	std::size_t ownPlayerIndex; //< Notre propre ID
};

void handle_message(const std::vector<std::uint8_t>& message, GameData& gameData);
bool run_network(ENetHost* host, GameData& gameData);


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

	// On envoie notre nom au serveur
	{
		PlayerNamePacket namePacket;
		namePacket.name = name;

		enet_peer_send(gameData.serverPeer, 0, build_packet(namePacket, ENET_PACKET_FLAG_RELIABLE));
	}

	Sel::Core core;

	Sel::Window window("Braaaaaawl", 1280, 720);
	Sel::Renderer renderer(window, SDL_RENDERER_PRESENTVSYNC);

	Sel::ResourceManager resourceManager(renderer);
	Sel::InputManager inputManager;

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

			Sel::InputManager::Instance().HandleEvent(event);
		}

		// On g�re la couche r�seau
		if (!run_network(host, gameData))
		{
			isOpen = false;
			break;
		}
	}

	// On pr�vient le serveur qu'on s'est d�connect�
	enet_peer_disconnect_now(gameData.serverPeer, 0);
	enet_host_flush(host);
	enet_host_destroy(host);

	enet_deinitialize();

	return EXIT_SUCCESS;
}

void handle_message(const std::vector<std::uint8_t>& message, GameData& gameData)
{
	// On d�code l'opcode pour savoir � quel type de message on a affaire
	std::size_t offset = 0;
	Opcode opcode = static_cast<Opcode>(Deserialize_u8(message, offset));
	switch (opcode)
	{

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