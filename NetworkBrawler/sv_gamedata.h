
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
	ENetPeer* peer = nullptr; //< Si peer est � nullptr alors le slot est libre
	std::size_t index; //< La position du joueur dans le tableau (sert d'id num�rique lors de l'affichage)
	std::string name; //< Nom du joueur
	std::optional<Brawler> brawler;
	std::optional<std::uint32_t> ownBrawlerNetworkId;
};

struct GameData
{
	GameData(entt::registry& reg) :
		registry(reg)
	{
	}

	Sel::Stopwatch clock;
	Sel::Stopwatch collectibleClock;
	float nextTick = 0.f;
	float tickInterval = TickDelay;

	float lastCollectibleSpawn = 0.f;
	float nextCollectibleSpawn = 2.0f;
	float collectibleSpawnInterval = 4.0f;
	std::uint32_t collectibleMaxCount = 25;

	std::vector<Player> players;
	entt::registry& registry;
	std::unordered_map<std::uint32_t, entt::handle> networkToEntity;
};