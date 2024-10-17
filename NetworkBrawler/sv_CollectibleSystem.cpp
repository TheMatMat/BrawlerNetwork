#include "sv_CollectibleSystem.h"
#include "sh_protocol.h"
#include "entt/entt.hpp"
#include <Sel/Transform.hpp>
#include <iostream>
#include "sv_networkedcomponent.h"
#include "sv_gamedata.h"

CollectibleSystem::CollectibleSystem(entt::registry& registry, GameData& gameData) :
	m_registry(registry),
    m_gameData(gameData)
{

}

bool CollectibleSystem::Update()
{
    bool collectionOccured = false;
	auto collectibleView = m_registry.view<Sel::Transform, CollectibleFlag, NetworkedComponent>();
	auto brawlerView = m_registry.view<Sel::Transform, BrawlerFlag, NetworkedComponent>();

    // Loop through all collectibles
    for (auto collectible : collectibleView)
    {
        // Get components for each collectible
        auto& collectibleTransform = collectibleView.get<Sel::Transform>(collectible);
        auto& collectibleNetwork = collectibleView.get<NetworkedComponent>(collectible);

        // Loop through all brawlers
        for (auto brawler : brawlerView)
        {
            // Get components for each brawler
            auto& brawlerTransform = brawlerView.get<Sel::Transform>(brawler);
            auto& brawlerNetwork = brawlerView.get<NetworkedComponent>(brawler);

            // Squared Distance
            Sel::Vector2f delta = brawlerTransform.GetPosition() - collectibleTransform.GetPosition();
            float distanceSqr = delta.x * delta.x + delta.y * delta.y;

            if (distanceSqr <= 50.f * 50.f)
            {
                // They are in contact
                //std::cout << "Brawler and Collectible are in contact!\n";

                // Remove the collectible from the registry
                m_registry.destroy(collectible);
                collectionOccured = true;

                // Find the player controlling the brawler and send him a packet to notify he got a collectible
                auto it = std::find_if(m_gameData.players.begin(), m_gameData.players.end(), [&](const Player& player) { return player.ownBrawlerNetworkId == brawlerNetwork.networkId; });
                if(it != m_gameData.players.end());
                {
                    Player& player = *it;

                    // Also update its score
                    player.playerScore++;

                    CollectibleCollectedPacket packet;
                    enet_peer_send(player.peer, 0, build_packet(packet, ENET_PACKET_FLAG_RELIABLE));
                }

                // Break out of the inner loop to stop checking other brawlers for this collectible
                break;
            }
        }
    }

    return collectionOccured;
}

