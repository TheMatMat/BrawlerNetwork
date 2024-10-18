#include "cl_brawler.h"
#include <Sel/Vector2.hpp>
#include <Sel/Sprite.hpp>
#include <Sel/Texture.hpp>

BrawlerClient::BrawlerClient(entt::registry& registry, Sel::Renderer& renderer) :
    BrawlerClient(registry, Sel::Vector2f(64.f, 64.f), 0.f, 1.f, Sel::Vector2f(5.f, 0.f))
{
    
}

BrawlerClient::BrawlerClient(entt::registry& registry, const Sel::Vector2f& position, float rotation, float scale, const Sel::Vector2f& linearVelocity) :
    Brawler(registry, position, rotation, scale, linearVelocity)
{
    if (m_handle)
    {
        // Load Ressources
        Sel::ResourceManager& resourceManager = Sel::ResourceManager::Instance();
        std::shared_ptr<Sel::Sprite> characterSprite = std::make_shared<Sel::Sprite>(BuildSprite(128.f, resourceManager));
        std::shared_ptr<Sel::Spritesheet> characterSpritesheet = resourceManager.GetSpritesheet("assets/characters/character.spritesheet");

        // Init Graphics Component
        auto& gfxComponent = m_handle.emplace<Sel::GraphicsComponent>();
        gfxComponent.renderable = characterSprite;

        // Init Spritesheet Component
        auto& spritesheetComponent = m_handle.emplace<Sel::SpritesheetComponent>(characterSpritesheet, characterSprite);
    }
}

Sel::Sprite BrawlerClient::BuildSprite(float size, Sel::ResourceManager& resourceManager)
{
    Sel::Sprite brawlerSprite(resourceManager.GetTexture("assets/characters/full_character2.png"));

    brawlerSprite.Resize(size, size);
    brawlerSprite.SetOrigin({ 0.5f, 0.5f });

    return brawlerSprite;
}