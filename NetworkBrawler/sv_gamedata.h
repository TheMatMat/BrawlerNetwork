
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
};

struct GameData
{
	GameData(entt::registry& reg) :
		registry(reg)
	{
	}

	Sel::Stopwatch clock;
	float nextTick = 0.f;
	float tickInterval = TickDelay;
	std::vector<Player> players;
	entt::registry& registry;
	std::unordered_map<std::uint32_t, entt::handle> networkToEntity;
};