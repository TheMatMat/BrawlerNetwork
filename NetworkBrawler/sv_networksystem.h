#pragma once

#include <entt/entt.hpp>
#include <enet6/enet.h>

struct GameData;

class NetworkSystem
{
public:
	NetworkSystem(entt::registry& registry, GameData& gameData);
	NetworkSystem(const NetworkSystem&) = delete;
	NetworkSystem(NetworkSystem&&) = delete;
	~NetworkSystem() = default;

	void CreateAllEntities(ENetPeer* peer);

	void Update();

	NetworkSystem& operator=(const NetworkSystem&) = delete;
	NetworkSystem& operator=(NetworkSystem&&) = delete;

private:
	void OnNetworkedConstruct(entt::registry& registry, entt::entity entity);
	void OnNetworkedDestruct(entt::registry& registry, entt::entity entity);

	entt::observer m_networkObserver;
	entt::registry& m_registry;
	GameData& m_gameData;
	std::uint32_t m_nextShapeId;
};