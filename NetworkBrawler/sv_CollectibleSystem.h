#pragma once

#include "entt/fwd.hpp";

struct GameData;

class CollectibleSystem
{
public:
	CollectibleSystem(entt::registry& registry, GameData& gameData);

	bool Update(GameData& gameData);

private:
	entt::registry& m_registry;
	GameData& m_gameData;
};