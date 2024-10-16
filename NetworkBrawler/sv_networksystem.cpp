#include "sv_networksystem.h"
#include "sv_networkedcomponent.h"
#include "sh_protocol.h"
#include "sh_constants.h"
#include "sv_gamedata.h"
#include <Sel/RigidBodyComponent.hpp>
#include <Sel/Transform.hpp>
#include <entt/entt.hpp>
#include <Sel/VelocityComponent.hpp>

NetworkSystem::NetworkSystem(entt::registry& registry, GameData& gameData) :
	m_networkObserver(registry, entt::collector.group<Sel::Transform, NetworkedComponent>()),
	m_registry(registry),
	m_gameData(gameData),
	m_nextShapeId(0)
{
	m_registry.on_construct<NetworkedComponent>().connect<&NetworkSystem::OnNetworkedConstruct>(this);
	m_registry.on_destroy<NetworkedComponent>().connect<&NetworkSystem::OnNetworkedDestruct>(this);
}

void NetworkSystem::CreateAllEntities(ENetPeer* peer)
{
	auto view = m_registry.view<NetworkedComponent, Sel::Transform>();
	for (entt::entity entity : view)
	{
		auto& transform = view.get<Sel::Transform>(entity);
		auto& networked = view.get<NetworkedComponent>(entity);

		if (m_registry.any_of<BrawlerFlag>(entity))
		{
			auto& velocity = m_registry.get<Sel::VelocityComponent>(entity);

			CreateBrawlerPacket createBrawler;
			createBrawler.brawlerId = networked.networkId;
			createBrawler.position = transform.GetPosition();
			createBrawler.linearVelocity = velocity.linearVel;
			createBrawler.scale = transform.GetScale().x;

			ENetPacket* createBrawlerPacket = build_packet(createBrawler, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, createBrawlerPacket);
		}
		else if (m_registry.any_of<CollectibleFlag>(entity))
		{
			auto& collectibleFlag = m_registry.get<CollectibleFlag>(entity);

			CreateCollectiblePacket createCollectible;
			createCollectible.collectibleId = networked.networkId;
			createCollectible.position = transform.GetPosition();
			createCollectible.scale = transform.GetScale().x;
			createCollectible.type = collectibleFlag.type;

			ENetPacket* createCollectiblePacket = build_packet(createCollectible, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, createCollectiblePacket);
		}
	}
}

void NetworkSystem::Update()
{
	m_networkObserver.each([&](entt::entity entity)
		{
			ENetPacket* createEntityPacket = nullptr;

			auto& transform = m_registry.get<Sel::Transform>(entity);
			auto& networked = m_registry.get<NetworkedComponent>(entity);

			if (m_registry.any_of<BrawlerFlag>(entity))
			{
				auto& velocity = m_registry.get<Sel::VelocityComponent>(entity);

				CreateBrawlerPacket createBrawler;
				createBrawler.brawlerId = networked.networkId;
				createBrawler.position = transform.GetPosition();
				createBrawler.linearVelocity = velocity.linearVel;
				createBrawler.scale = transform.GetScale().x;

				createEntityPacket = build_packet(createBrawler, ENET_PACKET_FLAG_RELIABLE);
			}
			else if (m_registry.any_of<CollectibleFlag>(entity))
			{
				auto& collectibleFlag = m_registry.get<CollectibleFlag>(entity);

				CreateCollectiblePacket createCollectible;
				createCollectible.collectibleId = networked.networkId;
				createCollectible.position = transform.GetPosition();
				createCollectible.scale = transform.GetScale().x;
				createCollectible.type = collectibleFlag.type;

				createEntityPacket = build_packet(createCollectible, ENET_PACKET_FLAG_RELIABLE);
			}

			if (!createEntityPacket)
				return;

			for (const Player& player : m_gameData.players)
			{
				if (player.peer != nullptr && !player.name.empty()) //< Est-ce que le slot est occupé par un joueur (et est-ce que ce joueur a bien envoyé son nom) ?
					enet_peer_send(player.peer, 0, createEntityPacket);
			}
		});

	auto view = m_registry.view<NetworkedComponent, Sel::Transform, Sel::VelocityComponent>();
	BrawlerStatesPacket brawlerStates;
	for (auto [entity, network, transform, velocity] : view.each())
	{
		auto& brawlerData = brawlerStates.brawlers.emplace_back();
		brawlerData.brawlerId = network.networkId;
		brawlerData.position = transform.GetPosition();
		brawlerData.linearVelocity = velocity.linearVel;
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