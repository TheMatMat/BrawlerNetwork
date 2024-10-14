#pragma once

#include <Sel/PhysicsSystem.hpp>
#include "sh_constants.h"
#include "sh_protocol.h"
#include "sv_gamedata.h"
#include "sv_networksystem.h"
#include <enet6/enet.h>
#include <entt/entt.hpp>
#include <iostream>
#include <cassert>
#include <Sel/Transform.hpp>

ENetPacket* build_playerlist_packet(GameData& gameData);

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData, NetworkSystem& networkSystem);
void tick(GameData& gameData, Sel::PhysicsSystem& physicsSystem, NetworkSystem& networkSystem);

int main()
{
	if (enet_initialize() != 0)
	{
		std::cout << "Failed to initialize ENet" << std::endl;
		return EXIT_FAILURE;
	}

	// Cr�ation de l'h�te serveur
	ENetAddress address;
	enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV6);
	address.port = AppPort;

	ENetHost* host = enet_host_create(ENET_ADDRESS_TYPE_ANY, &address, 10, 0, 0, 0);
	if (!host)
	{
		std::cerr << "Failed to create ENet host" << std::endl;
		return EXIT_FAILURE;
	}

	entt::registry registry;
	GameData gameData(registry);

	Sel::PhysicsSystem physicsSystem(registry);
	NetworkSystem networkSystem(registry, gameData);
	physicsSystem.SetGravity({ 0.f, 0.f });
	physicsSystem.SetDamping(0.9f);

	for (;;)
	{
		float now = gameData.clock.GetElapsedTime();

		ENetEvent event;
		if (enet_host_service(host, &event, 1) > 0) //< On met une milliseconde d'attente pour �viter que notre serveur ne bouffe tout le processeur
		{
			// On d�pile tous les �v�nements
			// enet_host_check_events permet de r�cup�rer les �v�nements sans faire tourner la couche r�seau d'ENet (la meilleure approche est donc d'appeler enet_host_service une fois puis enet_host_check_events tant que celle-ci renvoie une valeur sup�rieure � z�ro)

			do
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					// On enregistre le joueur en lui attribuant un id
					// On cherche ici le premier ID libre
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == nullptr; });
					if (it == gameData.players.end())
					{
						// Pas de slot libre, on en rajoute un
						auto& player = gameData.players.emplace_back();
						player.index = gameData.players.size() - 1;
						it = gameData.players.end() - 1; //< end() renvoie un it�rateur sur l'�l�ment apr�s le dernier �l�ment du tableau, on soustrait pour r�cup�rer un it�rateur sur le dernier �l�ment
					}

					Player& player = *it;
					player.color = Sel::Color{ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.f }; //< On associe une couleur al�atoire
					player.peer = event.peer; //< On associe le joueur � son peer
					player.name.clear();

					std::cout << "Player connected" << std::endl;
					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				{
					// Un joueur s'est d�connect�
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == event.peer; });
					assert(it != gameData.players.end());

					Player& player = *it;

					std::cout << "Player #" << player.index << " (" << player.name << ") disconnected from server" << std::endl;

					// On enl�ve son peer (pour rendre son slot disponible)
					player.peer = nullptr;

					// On renvoie la liste des joueurs � tous les joueurs (si ce joueur avait un nom)
					if (!player.name.empty())
					{
						ENetPacket* playerListPacket = build_playerlist_packet(gameData);

						for (const Player& player : gameData.players)
						{
							if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occup� par un joueur (et est-ce que ce joueur a bien envoy� son nom) ?
								enet_peer_send(player.peer, 0, playerListPacket);
						}
					}
					break;
				}

				case ENET_EVENT_TYPE_RECEIVE:
				{
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == event.peer; });
					assert(it != gameData.players.end());

					Player& player = *it;

					// On a re�u un message ! Traitons-le
					std::vector<std::uint8_t> content(event.packet->dataLength); //< On copie son contenu dans un std::vector pour plus de facilit� de gestion
					std::memcpy(content.data(), event.packet->data, event.packet->dataLength);

					// On g�re le message qu'on a re�u
					handle_message(player, content, gameData, networkSystem);

					// On n'oublie pas de lib�rer le packet
					enet_packet_destroy(event.packet);
					break;
				}
				}
			} while (enet_host_check_events(host, &event) > 0);
		}

		// On v�rifie si assez de temps s'est �coul� pour faire avancer la logique du jeu
		if (now >= gameData.nextTick)
		{
			//worldLimit.Update();

			// On met � jour la logique du jeu
			//tick(gameData, physicsSystem, networkSystem);

			// On pr�voit la prochaine mise � jour
			gameData.nextTick += gameData.tickInterval;
		}
	}

	return EXIT_SUCCESS;
}

ENetPacket* build_playerlist_packet(GameData& gameData)
{
	// Construisons le packet de liste de joueur
	PlayerListPacket packet;
	for (const Player& player : gameData.players)
	{
		if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occup� par un joueur (et est-ce que ce joueur a bien envoy� son nom) ?
		{
			// Oui, rajoutons-le � la liste
			auto& packetPlayer = packet.players.emplace_back();
			packetPlayer.color = player.color;
			packetPlayer.name = player.name;
		}
	}

	return build_packet(packet, ENET_PACKET_FLAG_RELIABLE);
}

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData, NetworkSystem& networkSystem)
{
	// On traite les messages re�us par un joueur, diff�renci�s par l'opcode
	std::size_t offset = 0;

	Opcode opcode = static_cast<Opcode>(Deserialize_u8(message, offset));
	switch (opcode)
	{
		case Opcode::C_PlayerName:
		{
			PlayerNamePacket playerName = PlayerNamePacket::Deserialize(message, offset);

			// On r�duit la taille du nom si elle est trop longue pour �viter les petits malins (oui je vous voir venir)
			if (playerName.name.size() > MaxPlayerNameLength)
				playerName.name.resize(MaxPlayerNameLength);

			std::cout << "Player #" << player.index << " is " << playerName.name << std::endl;
			player.name = playerName.name;

			// Envoyons la liste des joueurs
			ENetPacket* playerListPacket = build_playerlist_packet(gameData);

			for (const Player& player : gameData.players)
			{
				if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occup� par un joueur (et est-ce que ce joueur a bien envoy� son nom) ?
					enet_peer_send(player.peer, 0, playerListPacket);
			}

			// On cr�� toutes les entit�s de son c�t�
			networkSystem.CreateAllEntities(player.peer);

			break;
		}
	}
}



void tick(GameData& gameData, Sel::PhysicsSystem& physicsSystem, NetworkSystem& networkSystem)
{
	// On fait avancer le monde
	physicsSystem.Update(TickDelay);
	networkSystem.Update();

}