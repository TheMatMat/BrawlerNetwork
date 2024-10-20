#pragma once

#include "sh_constants.h"
#include <Sel/Color.hpp>
#include <Sel/Stopwatch.hpp>
#include <enet6/enet.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <entt/entity/handle.hpp>
#include "sh_brawler.h"

struct Player
{
	Sel::Color color; //< Couleur du joueur
	ENetPeer* peer = nullptr; //< Si peer est à nullptr alors le slot est libre
	std::size_t index; //< La position du joueur dans le tableau (sert d'id numérique lors de l'affichage)
	std::string name; //< Nom du joueur
	std::optional<Brawler> brawler;
	std::optional<std::uint32_t> ownBrawlerNetworkId;
	std::uint8_t playerScore = 0;
	bool isReady;
	bool isDead;
};

struct GoldenCarrot
{
	bool isSpawned = false;
	std::optional<std::uint32_t> owningBrawlerId;
	entt::handle handle;

	Sel::Stopwatch goldenCarrotClock;
	float spawnTime = 0.f;

	Sel::Stopwatch pointPulseClock; // < the golden carrot give 1 more points every 2 seconds to its owner
	float pulseTime = 3.f;
};

struct GameData
{
	GameData(entt::registry& reg, GoldenCarrot& _goldenCarrot) :
		registry(reg),
		goldenCarrot(_goldenCarrot)
	{
	}

	Sel::Stopwatch clock;
	Sel::Stopwatch collectibleClock;
	Sel::Stopwatch gameStartClock;
	Sel::Stopwatch killClock;

	float nextTick = 0.f;
	float tickInterval = TickDelay;

	float nextKill = 50.f;
	float killInterval = 50.f;

	GoldenCarrot& goldenCarrot;


	float lastCollectibleSpawn = 0.f;
	float nextCollectibleSpawn = 0.5f;
	float collectibleSpawnInterval = 4.0f;
	std::uint32_t collectibleMaxCount = 25;

	GameState gamesState = GameState::Lobby;
	bool allReady = false;

	Player* lastWinner = nullptr;
	std::vector<Player> players;
	std::vector<Player*> playingPlayers; // players in the game. filled at game start with players present in lobby
	std::vector<Player*> leaderBoard; // reordered when score changes first is the highest score
	entt::registry& registry;
	std::unordered_map<std::uint32_t, entt::handle> networkToEntity;
};