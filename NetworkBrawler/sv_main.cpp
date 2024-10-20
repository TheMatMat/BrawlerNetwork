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
#include "sh_brawler.h"
#include <Sel/VelocitySystem.hpp>
#include "sv_networkedcomponent.h"
#include <random>
#include "sv_CollectibleSystem.h"
#include <Sel/VelocityComponent.hpp>

ENetPacket* build_playerlist_packet(GameData& gameData);

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData, NetworkSystem& networkSystem);
void tick(GameData& gameData, Sel::PhysicsSystem& physicsSystem, Sel::VelocitySystem& velocitySystem, NetworkSystem& networkSystem, CollectibleSystem& collectibleSystem);
entt::handle spawn_collectible(GameData& gameData, const CollectibleType& type = CollectibleType::Carrot);
void start_game(GameData& gameData);
void end_game(GameData& gameData);
void update_leaderboard(GameData& gameData);

int main()
{
	if (enet_initialize() != 0)
	{
		std::cout << "Failed to initialize ENet" << std::endl;
		return EXIT_FAILURE;
	}

	// Création de l'hôte serveur
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
	GoldenCarrot goldenCarrot;
	GameData gameData(registry, goldenCarrot);

	Sel::PhysicsSystem physicsSystem(registry);
	Sel::VelocitySystem velocitySystem(registry);
	NetworkSystem networkSystem(registry, gameData);
	CollectibleSystem collectibleSystem(registry, gameData);
	physicsSystem.SetGravity({ 0.f, 0.f });
	physicsSystem.SetDamping(0.9f);

	for (;;)
	{
		float now = gameData.clock.GetElapsedTime();
		float nowCollectible = gameData.collectibleClock.GetElapsedTime();
		float nowKill = gameData.killClock.GetElapsedTime();

		ENetEvent event;
		if (enet_host_service(host, &event, 1) > 0) //< On met une milliseconde d'attente pour éviter que notre serveur ne bouffe tout le processeur
		{
			// On dépile tous les événements
			// enet_host_check_events permet de récupérer les événements sans faire tourner la couche réseau d'ENet (la meilleure approche est donc d'appeler enet_host_service une fois puis enet_host_check_events tant que celle-ci renvoie une valeur supérieure à zéro)

			do
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					if (gameData.players.empty()) {
						// Reserve space for players (you could adjust this based on your needs)
						gameData.players.reserve(25);
					}

					// If the players vector is already full, refuse the connection
					if (gameData.players.size() >= gameData.players.capacity()) {
						std::cout << "Connection refused: player limit reached" << std::endl;
						enet_peer_disconnect_now(event.peer, 0); // Disconnect the peer immediately
						break; // Exit the connection handling
					}

					// On enregistre le joueur en lui attribuant un id
					// On cherche ici le premier ID libre
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == nullptr; });
					if (it == gameData.players.end())
					{
						// Pas de slot libre, on en rajoute un
						auto& player = gameData.players.emplace_back();
						player.index = gameData.players.size() - 1;
						it = gameData.players.end() - 1; //< end() renvoie un itérateur sur l'élément après le dernier élément du tableau, on soustrait pour récupérer un itérateur sur le dernier élément
					}

					Player& player = *it;
					player.color = Sel::Color{ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.f }; //< On associe une couleur aléatoire
					player.peer = event.peer; //< On associe le joueur à son peer
					player.name.clear();

					// Someone join so he is not ready. Stop the game start countdown
					if (gameData.gamesState == GameState::Lobby)
					{
						gameData.allReady = false;
					}


					std::cout << "Player connected" << std::endl;
					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT:
				case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				{
					// Un joueur s'est déconnecté
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == event.peer; });
					assert(it != gameData.players.end());

					Player& player = *it;

					std::cout << "Player #" << player.index << " (" << player.name << ") disconnected from server" << std::endl;

					// On enlève son peer (pour rendre son slot disponible)
					player.peer = nullptr;

					// On delete son brawler
					player.brawler.reset();
					if (player.ownBrawlerNetworkId)
					{
						auto it = gameData.networkToEntity.find(*(player.ownBrawlerNetworkId));
						if (it != gameData.networkToEntity.end())
						{
							entt::handle entityHandle = it->second;
							gameData.registry.destroy(entityHandle);

							gameData.networkToEntity.erase(it);

						}
					}
					player.ownBrawlerNetworkId.reset();

					// Supprimer le joueur de gameData.playingPlayers et gameData.leaderBoard
					gameData.playingPlayers.erase(
						std::remove(gameData.playingPlayers.begin(), gameData.playingPlayers.end(), &player),
						gameData.playingPlayers.end()
					);

					gameData.leaderBoard.erase(
						std::remove(gameData.leaderBoard.begin(), gameData.leaderBoard.end(), &player),
						gameData.leaderBoard.end()
					);


					// On renvoie la liste des joueurs à tous les joueurs (si ce joueur avait un nom)
					if (!player.name.empty())
					{
						ENetPacket* playerListPacket = build_playerlist_packet(gameData);

						for (const Player& player : gameData.players)
						{
							if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occupé par un joueur (et est-ce que ce joueur a bien envoyé son nom) ?
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

					// On a reçu un message ! Traitons-le
					std::vector<std::uint8_t> content(event.packet->dataLength); //< On copie son contenu dans un std::vector pour plus de facilité de gestion
					std::memcpy(content.data(), event.packet->data, event.packet->dataLength);

					// On gère le message qu'on a reçu
					handle_message(player, content, gameData, networkSystem);

					// On n'oublie pas de libérer le packet
					enet_packet_destroy(event.packet);
					break;
				}
				}
			} while (enet_host_check_events(host, &event) > 0);
		}
		

		// On vérifie si assez de temps s'est écoulé pour faire avancer la logique du jeu
		if (now >= gameData.nextTick)
		{
			//worldLimit.Update();

			// On met à jour la logique du jeu
			tick(gameData, physicsSystem, velocitySystem, networkSystem, collectibleSystem);

			if (gameData.gamesState == GameState::GameRunning)
			{
				auto brawlerView = gameData.registry.view<BrawlerFlag>(entt::exclude<DeadFlag>);
				int brawlerCount = std::distance(brawlerView.begin(), brawlerView.end());

				if (brawlerCount <= 1)
				{
					// There's only one brawler left, find it
					for (auto entity : brawlerView)
					{
						// Get the NetworkComponent of the last remaining brawler
						auto& networkComp = gameData.registry.get<NetworkedComponent>(entity);
						auto lastNetworkID = networkComp.networkId;

						// Find the player whose ownBrawlerNetworkID matches lastNetworkID
						auto it = std::find_if(gameData.playingPlayers.begin(), gameData.playingPlayers.end(),
							[&lastNetworkID](Player* player)
							{
								return player->ownBrawlerNetworkId == lastNetworkID;
							});

						if (it != gameData.playingPlayers.end())
						{
							// Store the last winner in gameData
							gameData.lastWinner = *it;

							std::cout << gameData.lastWinner->name << " wins -> END GAME" << std::endl;
						}
					}

					gameData.gamesState = GameState::EndScreen;
					end_game(gameData);
				}
				else
				{
					// Spawn de la carotte legendaire
					if (brawlerCount > 0 && gameData.goldenCarrot.goldenCarrotClock.GetElapsedTime() >= gameData.goldenCarrot.spawnTime && !gameData.goldenCarrot.isSpawned)
					{
						gameData.goldenCarrot.handle = spawn_collectible(gameData, CollectibleType::GoldenCarrot);
						gameData.goldenCarrot.isSpawned = true;
					}

					// Add point to the brawler owning the golden carrot
					if (
						brawlerCount > 0
						&& gameData.goldenCarrot.isSpawned
						&& gameData.goldenCarrot.owningBrawlerId.has_value()
						&& gameData.goldenCarrot.pointPulseClock.GetElapsedTime() >= gameData.goldenCarrot.pulseTime
						)
					{
						// Find the player controlling the brawler and send him a packet to notify he got a collectible
						auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.ownBrawlerNetworkId == gameData.goldenCarrot.owningBrawlerId; });
						if (it != gameData.players.end());
						{
							Player& player = *it;

							// Update its score
							player.playerScore++;
						}
						gameData.goldenCarrot.pointPulseClock.Restart();

						update_leaderboard(gameData);
					}

					std::size_t collectibleCount = gameData.registry.view<CollectibleFlag>().size();

					// On check s'il y a au moins un brawler et si le nombre de collectibles est inferieur au maximum autorise
					if (brawlerCount > 0 && collectibleCount < gameData.collectibleMaxCount)
					{
						if (now >= gameData.nextCollectibleSpawn)
						{
							spawn_collectible(gameData);
							/*std::cout << "Spawn Collectible now - " << collectibleCount + 1 << std::endl;*/
							gameData.nextCollectibleSpawn = now + gameData.collectibleSpawnInterval;  // Prochain spawn X seconds apres mtnt
						}
					}

					// System qui kill le dernier à interval régulier
					if (brawlerCount > 0 && nowKill >= gameData.nextKill)
					{

						for (auto it = gameData.leaderBoard.rbegin(); it != gameData.playingPlayers.rend(); ++it)
						{
							if ((*it)->isDead)
								continue;

							// Set the player as dead
							(*it)->isDead = true;

							std::cout << (*it)->name << "'s brawler has been killed" << std::endl;

							if (!(*it)->ownBrawlerNetworkId)
								continue;

							// Find the brawler associated with this player in the NetworkToEntities map
							auto entityIt = gameData.networkToEntity.find((*it)->ownBrawlerNetworkId.value());
							if (entityIt != gameData.networkToEntity.end())
							{
								// Add DeadFlag to the entity
								gameData.registry.emplace_or_replace<DeadFlag>(entityIt->second);

								Sel::Vector2f deathPosition;
								std::int8_t deathScaleX = 1.f;
								auto transform = gameData.registry.try_get<Sel::Transform>(entityIt->second);
								if (transform)
								{
									deathPosition = transform->GetGlobalPosition();
									deathScaleX = transform->GetScale().x;
									transform->SetPosition({ -20000.f, -20000.f }); // On le place très loin
								}

								update_leaderboard(gameData);

								// On notifie tout les joueurs de cette mort
								BrawlerDeathPacket packet;
								packet.playerId = (*it)->index;
								packet.brawlerId = (*it)->ownBrawlerNetworkId.value();
								packet.deathPosition = deathPosition;
								packet.deathScaleX = deathScaleX;

								for (auto& player : gameData.playingPlayers)
								{
									if (!player->peer)
										continue;

									enet_peer_send(player->peer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
								}
							}

							break;
						}

						gameData.nextKill = nowKill + gameData.killInterval;
					}


				}


				
			}

			// On prévoit la prochaine mise à jour
			gameData.nextTick += gameData.tickInterval;
		}	

		// Countdown until game starts when all brawlers are ready
		float nowStartGameCountdown = gameData.gameStartClock.GetElapsedTime();
		if (gameData.gamesState == GameState::Lobby && gameData.allReady && nowStartGameCountdown >= 5.0f)
		{
			gameData.allReady = false;
			gameData.gamesState = GameState::GameRunning;

			start_game(gameData);

			gameData.killClock.Restart();
			std::cout << "START GAME!" << std::endl;


			UpdateGameStatePacket packet;
			packet.newGameState = static_cast<std::uint8_t>(gameData.gamesState);

			for (const auto& player : gameData.playingPlayers)
			{
				if (!player->peer)
					continue;

				enet_peer_send(player->peer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
			}
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
		if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occupé par un joueur (et est-ce que ce joueur a bien envoyé son nom) ?
		{
			// Oui, rajoutons-le à la liste
			auto& packetPlayer = packet.players.emplace_back();
			packetPlayer.name = player.name;
			if (player.ownBrawlerNetworkId.has_value())
			{
				packetPlayer.id = player.index;
				packetPlayer.hasBrawler = true;
				packetPlayer.isDead = player.isDead;
				packetPlayer.brawlerId = player.ownBrawlerNetworkId.value();
			}
			else
			{
				packetPlayer.id = player.index;
				packetPlayer.hasBrawler = false;
				packetPlayer.isDead = player.isDead;
				packetPlayer.brawlerId.reset();
			}
		}
	}

	return build_packet(packet, ENET_PACKET_FLAG_RELIABLE);
}

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData, NetworkSystem& networkSystem)
{
	// On traite les messages reçus par un joueur, différenciés par l'opcode
	std::size_t offset = 0;

	Opcode opcode = static_cast<Opcode>(Deserialize_u8(message, offset));
	switch (opcode)
	{
		case Opcode::C_PlayerName:
		{
			PlayerNamePacket playerName = PlayerNamePacket::Deserialize(message, offset);

			// On réduit la taille du nom si elle est trop longue pour éviter les petits malins (oui je vous voir venir)
			if (playerName.name.size() > MaxPlayerNameLength)
				playerName.name.resize(MaxPlayerNameLength);

			std::cout << "Player #" << player.index << " is " << playerName.name << std::endl;
			player.name = playerName.name;

			// Envoyons la liste des joueurs
			ENetPacket* playerListPacket = build_playerlist_packet(gameData);

			for (const Player& player : gameData.players)
			{
				if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occupé par un joueur (et est-ce que ce joueur a bien envoyé son nom) ?
					enet_peer_send(player.peer, 0, playerListPacket);
			}

			// On créé toutes les entités de son côté
			networkSystem.CreateAllEntities(player.peer);

			// On lui envoie l'état du jeu et par conséquent son player mode
			UpdateGameStatePacket gameStatePacket;
			gameStatePacket.newGameState = static_cast<std::uint8_t>(gameData.gamesState);

			UpdatePlayerModePacket playerModePacket;
			if (gameData.gamesState == GameState::Lobby)
			{
				player.isDead = false;
				playerModePacket.newPlayerMode = static_cast<std::uint8_t>(PlayerMode::Playing);
			}
			else
			{
				player.isDead = true;
				playerModePacket.newPlayerMode = static_cast<std::uint8_t>(PlayerMode::Spectating);
			}

			enet_peer_send(player.peer, 0, build_packet(playerModePacket, ENET_PACKET_FLAG_RELIABLE));
			//enet_peer_send(player.peer, 0, build_packet(gameStatePacket, ENET_PACKET_FLAG_RELIABLE));

			break;
		}
		case Opcode::C_CreateBrawlerRequest:
		{
			std::cout << "Player " << player.name << " wants to spawn its brawler" << std::endl;

			// On cree le brawler coté serveur
			Brawler brawler(gameData.registry, Sel::Vector2f(100.f, 20.f), 0.f, 1.f, Sel::Vector2f(10.f, 0.f));


			// on lui donne l'id de son player
			auto flag = brawler.GetHandle().try_get<BrawlerFlag>();
			if (flag)
			{
				flag->playerId = player.index;
			}

			auto network = brawler.GetHandle().try_get<NetworkedComponent>();
			if (!network)
				break;

			// On l'ajoute à la liste d'entité
			gameData.networkToEntity[network->networkId] = brawler.GetHandle();

			// On renvois au createur l'id réseaux de son brawler
			UpdateSelfBrawlerId updateSelfBrawlerIdPacket;
			updateSelfBrawlerIdPacket.id = network->networkId;

			enet_peer_send(player.peer, 0, build_packet(updateSelfBrawlerIdPacket, ENET_PACKET_FLAG_RELIABLE));

			player.ownBrawlerNetworkId = network->networkId;
			player.brawler = std::move(brawler);

			break;
		}
		case Opcode::C_PlayerInputs:
		{
			PlayerInputsPacket packet = PlayerInputsPacket::Deserialize(message, offset);

			if (!player.brawler)
				break;

			// On applique ses input si son brawler n'est pas mort
			if(!player.isDead && gameData.gamesState != GameState::EndScreen)
				player.brawler->ApplyInputs(packet.inputs);


			break;
		}
		case Opcode::C_PlayerReady:
		{
			PlayerReadyPacket packet = PlayerReadyPacket::Deserialize(message, offset);

			if (gameData.gamesState == GameState::EndScreen)
			{
				gameData.gamesState = GameState::Lobby;

				// On ramène tout le monde au centre
				auto view = gameData.registry.view<Sel::Transform, BrawlerFlag>();
				for (auto&& [entity, transform, flag] : view.each())
				{
					transform.SetPosition(Sel::Vector2f(0.f, 0.f));

					if (gameData.registry.all_of<DeadFlag>(entity))
						gameData.registry.remove<DeadFlag>(entity);
				}

				UpdateGameStatePacket packet;
				packet.newGameState = static_cast<std::uint8_t>(gameData.gamesState);
				
				for (auto& player : gameData.players)
				{
					if (!player.peer)
						continue;
							
					player.isReady = false;
					player.isDead = false;
					enet_peer_send(player.peer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
				}

				break;
			}

			std::cout << "player " << player.index << " ready state is: " << static_cast<int>(packet.newReadyValue) << std::endl;
			player.isReady = packet.newReadyValue;


			// Qqn se met pas prêt
			if (!packet.newReadyValue)
			{
				gameData.allReady = false;
				break;
			}

			bool allReady = true;
			for (const auto& player : gameData.players)
			{
				if (!player.peer)
				{
					continue;
				}

				if (player.isReady)
					continue;

				allReady = false;
				break;
			}

			if (allReady)
			{
				gameData.gameStartClock.Restart();
				gameData.allReady = true;
			}

			break;
		}
	}
}



void tick(GameData& gameData, Sel::PhysicsSystem& physicsSystem, Sel::VelocitySystem& velocitySystem, NetworkSystem& networkSystem, CollectibleSystem& collectibleSystem)
{
	// On fait avancer le monde
	//physicsSystem.Update(TickDelay);
	velocitySystem.Update(TickDelay);
	networkSystem.Update();

	if (gameData.gamesState == GameState::GameRunning)
	{
		// Update the collectible system and modify leaderbaord if one collection occured (return true) 
		if (collectibleSystem.Update())
		{
			std::cout << "update leaderboard" << std::endl;

			update_leaderboard(gameData);
		}
	}
}

entt::handle spawn_collectible(GameData& gameData, const CollectibleType& type)
{
	entt::entity newCollectible = gameData.registry.create();

	// random spawn position
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> disX(WORLD_MIN_X, WORLD_MAX_X);
	std::uniform_real_distribution<float> disY(WORLD_MIN_Y, WORLD_MAX_Y);

	float spawnX = disX(gen);
	float spawnY = disY(gen);

	auto& transform = gameData.registry.emplace<Sel::Transform>(newCollectible);
	if(type == CollectibleType::GoldenCarrot)
		transform.SetPosition({ 0.f, 0.f });
	else
		transform.SetPosition({ spawnX, spawnY });
	transform.SetRotation(0.f);
	transform.SetScale({ 0.5f, 0.5f });

	// The server synchronize position of the entity with velocity component. 
	// Sometimes the golden carrot is moved in/out the game space when it's catch/released instead of instantiate/destroyed
	// So I add it that velocity component even if it is not really moving so that in/out get sync on client 
	// That might be not clean
	if (type == CollectibleType::GoldenCarrot) 
	{
		auto& velocity = gameData.registry.emplace<Sel::VelocityComponent>(newCollectible);
		velocity.linearVel = { 0.f, 0.f };
	}

	auto& network = gameData.registry.emplace<NetworkedComponent>(newCollectible);

	auto& collectibleFlag = gameData.registry.emplace<CollectibleFlag>(newCollectible);
	collectibleFlag.type = type;

	if(type == CollectibleType::GoldenCarrot)
		gameData.registry.emplace<GoldenCarrotFlag>(newCollectible);

	// On crée un handle
	entt::handle handle = entt::handle(gameData.registry, newCollectible);

	// On l'ajoute à la liste d'entité
	gameData.networkToEntity[network.networkId] = handle;

	return handle;
}

void start_game(GameData& gameData)
{
	gameData.playingPlayers.clear();
	gameData.leaderBoard.clear();

	for (auto& player : gameData.players)
	{
		if (player.name.empty() || !player.peer || !player.ownBrawlerNetworkId)
			continue;

		player.playerScore = 0;
		player.isDead = false;

		gameData.playingPlayers.push_back(&player);
		gameData.leaderBoard.push_back(&player);
	}

	// float goldenCarrotSpawnTime = static_cast<int>(gameData.playingPlayers.size() * 0.5f) * gameData.killInterval + 4.0f;
	float goldenCarrotSpawnTime = 5.f;

	gameData.goldenCarrot.goldenCarrotClock.Restart();
	gameData.goldenCarrot.spawnTime = goldenCarrotSpawnTime;

	gameData.killClock.Restart();
	gameData.nextKill = gameData.killInterval;

	update_leaderboard(gameData);
}

void end_game(GameData& gameData)
{
	// destroy all collectibles remaining
	auto collectiblesView = gameData.registry.view<CollectibleFlag>();
	for (auto collectible : collectiblesView)
	{
		gameData.registry.destroy(collectible);
	}

	// stop movement in case of need (i.e. brawler has been killed but last input received indicate it has to move)
	auto view = gameData.registry.view<Sel::VelocityComponent>();
	for (auto&& [entity, velocity] : view.each())
	{
		velocity.linearVel = Sel::Vector2f(0.f, 0.f);
	}

	// Notifions tout le monde qu'il y a un gagnant
	WinnerPacket packet;
	packet.brawlerNetworkId = gameData.lastWinner->index;

	UpdateGameStatePacket gameStatePacket;
	gameStatePacket.newGameState = static_cast<std::uint8_t>(gameData.gamesState);

	for (auto& player : gameData.playingPlayers)
	{
		if (!player->peer)
			continue;

		enet_peer_send(player->peer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
		enet_peer_send(player->peer, 0, build_packet(gameStatePacket, ENET_PACKET_FLAG_RELIABLE));
	}

}

void update_leaderboard(GameData& gameData)
{
	std::stable_sort(gameData.leaderBoard.begin(), gameData.leaderBoard.end(),
		[](const Player* a, const Player* b)
		{
			if (a->isDead != b->isDead) {
				return !a->isDead; // Les joueurs morts vont à la fin
			}
			return a->playerScore > b->playerScore; // Tri par score décroissant
		});

	// On notifie tout le monde du changement de leadearboard
	UpdateLeaderboardPacket packet;

	for (auto& playerData : gameData.leaderBoard)
	{
		auto& data = packet.leaderboard.emplace_back();
		data.playerId = playerData->index;
		data.playerName = playerData->name;
		data.playerScore = playerData->playerScore;
		data.isDead = playerData->isDead;
	}

	for (auto& player : gameData.players)
	{
		if (!player.peer)
			continue;

		enet_peer_send(player.peer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
	}
}
