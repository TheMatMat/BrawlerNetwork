#include "sh_brawler.h"
#include <Sel/Transform.hpp>
#include <Sel/RigidBodyComponent.hpp>
#include <Sel/ResourceManager.hpp>
#include "sv_networkedcomponent.h"
#include <Sel/Vector2.hpp>
#include <Sel/VelocityComponent.hpp>
#include "sh_inputs.h"


Brawler::Brawler(entt::registry& registry) :
    Brawler(registry, Sel::Vector2f(64.f, 64.f), 0.f, 1.f, Sel::Vector2f(5.f, 0.f))
{

}

Brawler::Brawler(entt::registry& registry, const Sel::Vector2f& position, float rotation, float scale, const Sel::Vector2f& linearVelocity) :
    m_position(position),
    m_linearVelocity(linearVelocity),
    m_speed(100.f)
{
    entt::entity brawler = registry.create();

    // Init Transform Component
    auto& transform = registry.emplace<Sel::Transform>(brawler);
    transform.SetPosition(position);
    transform.SetRotation(rotation);

    transform.SetScale({scale, scale});

    // Init RigidBody Component
    /*auto& rigidBody = registry.emplace<Sel::RigidBodyComponent>(brawler, 0);
    rigidBody.TeleportTo(position, rotation);*/

    // Init Velocity Component
    auto& velocityComponent = registry.emplace<Sel::VelocityComponent>(brawler);
    velocityComponent.linearVel = linearVelocity;

    // Init Network Component
    registry.emplace<NetworkedComponent>(brawler);

    m_handle = entt::handle(registry, brawler);
}

const Sel::Vector2f& Brawler::GetPosition() const
{
    if (!m_handle)
        throw std::runtime_error("entity invalid");

    const auto& transform = m_handle.try_get<Sel::Transform>();

    if(!transform)
        throw std::runtime_error("entity has no transform");

    return transform->GetGlobalPosition();
}

const Sel::Vector2f& Brawler::GetVelocity() const
{
    if (!m_handle)
        throw std::runtime_error("entity invalid");

    const auto& velocityComponent = m_handle.try_get<Sel::VelocityComponent>();

    if (!velocityComponent)
        throw std::runtime_error("entity has no velocity component");

    return velocityComponent->linearVel;
}

const entt::handle& Brawler::GetHandle()
{
    return m_handle;
}

std::uint32_t Brawler::GetId()
{
    if (!m_handle)
        throw std::runtime_error("entity invalid");

    const auto& network = m_handle.try_get<NetworkedComponent>();

    if (!network)
        throw std::runtime_error("entity has no network component");

    return network->networkId;
}

void Brawler::ApplyInputs(const PlayerInputs& inputs)
{
    if (!m_handle)
        throw std::runtime_error("entity invalid");

    const auto& velocityComponent = m_handle.try_get<Sel::VelocityComponent>();

    if (!velocityComponent)
        throw std::runtime_error("entity has no velocity component");

    Sel::Vector2f newLinearVelocity(0.f, 0.f);

    if (inputs.moveLeft)
        newLinearVelocity -= Sel::Vector2f(m_speed, 0.f);
    if (inputs.moveRight)
        newLinearVelocity += Sel::Vector2f(m_speed, 0.f);
    if (inputs.moveUp)
        newLinearVelocity -= Sel::Vector2f(0.f, m_speed);
    if (inputs.moveDown)
        newLinearVelocity += Sel::Vector2f(0.f, m_speed);

    // Normalize velocity if moving diagonally
    if (newLinearVelocity.x != 0.f && newLinearVelocity.y != 0.f)
    {
        newLinearVelocity = Sel::Vector2f::Normal(newLinearVelocity) * m_speed;
    }

    velocityComponent->linearVel = newLinearVelocity;
}
