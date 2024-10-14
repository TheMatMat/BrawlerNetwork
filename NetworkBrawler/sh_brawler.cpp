#include "sh_brawler.h"
#include <Sel/Transform.hpp>
#include <Sel/RigidBodyComponent.hpp>
#include "sv_networkedcomponent.h"


Brawler::Brawler(entt::registry& registry, const Sel::Vector2f& position) : 
    m_position(position),
    m_linearVelocity(0.f, 0.f)
{
    entt::entity brawler = registry.create();

    // Init Transform Component
    auto& transform = registry.emplace<Sel::Transform>(brawler);
    transform.SetPosition(position);
    float rotation = 0.f;
    transform.SetRotation(rotation);

    // Init RigidBody Component
    auto& rigidBody = registry.emplace<Sel::RigidBodyComponent>(brawler, 0);
    rigidBody.TeleportTo(position, rotation);

    // Init Network Component
    registry.emplace<NetworkedComponent>(brawler);

    m_handle = entt::handle(registry, brawler);
}

const Sel::Vector2f& Brawler::GetPosition() const
{
    return m_position;
}

const Sel::Vector2f& Brawler::GetVelocity() const
{
    return m_linearVelocity;
}
