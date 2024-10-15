#include "cl_brawler.h"
#include <Sel/Vector2.hpp>
#include <Sel/Sprite.hpp>
#include <Sel/Texture.hpp>

BrawlerClient::BrawlerClient(entt::registry& registry) :
    BrawlerClient(registry, Sel::Vector2f(64.f, 64.f), 0.f, 1.f, Sel::Vector2f(5.f, 0.f))
{
    
}

BrawlerClient::BrawlerClient(entt::registry& registry, const Sel::Vector2f& position, float rotation, float scale, const Sel::Vector2f& linearVelocity) :
    Brawler(registry, position, rotation, scale, linearVelocity)
{
    // Init Graphics Component
    if (m_handle)
    {
        auto& gfxComponent = m_handle.emplace<Sel::GraphicsComponent>();
        gfxComponent.renderable = std::make_shared<Sel::Sprite>(BuildSprite(128.f));
    }
}

Sel::Sprite BrawlerClient::BuildSprite(float scale)
{
    Sel::ResourceManager& resourceManager = Sel::ResourceManager::Instance();
    Sel::Sprite brawlerSprite(resourceManager.GetTexture("assets/ball.png"));

    brawlerSprite.Resize(scale, scale);
    brawlerSprite.SetOrigin({ 0.5f, 0.5f });

    return brawlerSprite;
}