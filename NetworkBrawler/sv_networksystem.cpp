#include "sv_networksystem.h"
#include "sv_networkedcomponent.h"
#include "sh_protocol.h"
#include "sv_gamedata.h"
#include <Sel/RigidBodyComponent.hpp>
#include <Sel/Transform.hpp>
#include <entt/entt.hpp>

NetworkSystem::NetworkSystem(entt::registry& registry, GameData& gameData) :
	m_networkObserver(registry, entt::collector.group<Sel::Transform, NetworkedComponent, Sel::RigidBodyComponent>()),
	m_registry(registry),
	m_gameData(gameData),
	m_nextShapeId(0)
{
	m_registry.on_construct<NetworkedComponent>().connect<&NetworkSystem::OnNetworkedConstruct>(this);
	m_registry.on_destroy<NetworkedComponent>().connect<&NetworkSystem::OnNetworkedDestruct>(this);
}

void NetworkSystem::CreateAllEntities(ENetPeer* peer)
{
	auto view = m_registry.view<NetworkedComponent, Sel::Transform, Sel::RigidBodyComponent, BrawlerData>();
	for (entt::entity entity : view)
	{
		auto& transform = view.get<Sel::Transform>(entity);
		auto& networked = view.get<NetworkedComponent>(entity);
		auto& shapeData = view.get<BrawlerData>(entity);

		CreateBrawlerPacket createBrawler;
		createBrawler.brawlerData.position = transform.GetPosition();

		ENetPacket* createShapePacket = build_packet(createBrawler, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, createShapePacket);
	}
}

void NetworkSystem::Update()
{
	m_networkObserver.each([&](entt::entity entity)
		{
			auto& transform = m_registry.get<Sel::Transform>(entity);
			auto& networked = m_registry.get<NetworkedComponent>(entity);
			auto& brawlerData = m_registry.get<BrawlerData>(entity);

			CreateBrawlerPacket createBrawler;
			createBrawler.brawlerData.position = transform.GetPosition();

			ENetPacket* createShapePacket = build_packet(createBrawler, ENET_PACKET_FLAG_RELIABLE);

			for (const Player& player : m_gameData.players)
			{
				if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occupé par un joueur (et est-ce que ce joueur a bien envoyé son nom) ?
					enet_peer_send(player.peer, 0, createShapePacket);
			}
		});

	auto view = m_registry.view<NetworkedComponent, Sel::Transform, Sel::RigidBodyComponent>();
	BrawlerStatesPacket brawlerStates;
	for (auto [entity, network, transform, rigidBody] : view.each())
	{
		auto& brawlerData = brawlerStates.brawlers.emplace_back();
		brawlerData.brawlerId = network.networkId;
		brawlerData.position = transform.GetPosition();
		brawlerData.linearVelocity = rigidBody.GetLinearVelocity();
	}

	ENetPacket* brawlerStatesPacket = build_packet(brawlerStates, 0);

	for (const Player& player : m_gameData.players)
	{
		if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occupé par un joueur (et est-ce que ce joueur a bien envoyé son nom) ?
			enet_peer_send(player.peer, 0, brawlerStatesPacket);
	}
}

void NetworkSystem::OnNetworkedConstruct(entt::registry& registry, entt::entity entity)
{
	auto& networkedComponent = registry.get<NetworkedComponent>(entity);
	networkedComponent.networkId = m_nextShapeId++;
}

void NetworkSystem::OnNetworkedDestruct(entt::registry& registry, entt::entity entity)
{
	auto& networked = m_registry.get<NetworkedComponent>(entity);

	DeleteBrawlerPacket deleteBrawler;
	deleteBrawler.brawlerId = networked.networkId;

	ENetPacket* deleteShapePacket = build_packet(deleteBrawler, ENET_PACKET_FLAG_RELIABLE);

	for (const Player& player : m_gameData.players)
	{
		if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occupé par un joueur (et est-ce que ce joueur a bien envoyé son nom) ?
			enet_peer_send(player.peer, 0, deleteShapePacket);
	}
}