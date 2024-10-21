#include "cl_brawler.h"
#include <Sel/Vector2.hpp>
#include <Sel/Sprite.hpp>
#include <Sel/Texture.hpp>
#include "sh_temporaryEntitySystem.h"

BrawlerClient::BrawlerClient(entt::registry& registry, Sel::Renderer& renderer) :
    BrawlerClient(registry, Sel::Vector2f(64.f, 64.f), 0.f, 1.f, Sel::Vector2f(5.f, 0.f))
{
    
}

BrawlerClient::BrawlerClient(entt::registry& registry, const Sel::Vector2f& position, float rotation, float scale, const Sel::Vector2f& linearVelocity, int skindId) :
    Brawler(registry, position, rotation, scale, linearVelocity)
{
    if (m_handle)
    {
        // Load Ressources
        Sel::ResourceManager& resourceManager = Sel::ResourceManager::Instance();
        std::shared_ptr<Sel::Sprite> characterSprite = std::make_shared<Sel::Sprite>(BuildSprite(128.f, resourceManager, skindId));
        std::shared_ptr<Sel::Spritesheet> characterSpritesheet = resourceManager.GetSpritesheet("assets/characters/character.spritesheet");

        // Init Spritesheet Component
        auto& spritesheetComponent = m_handle.emplace<Sel::SpritesheetComponent>(characterSpritesheet, characterSprite);
        
        // Init Graphics Component
        auto& gfxComponent = m_handle.emplace<Sel::GraphicsComponent>();
        gfxComponent.renderable = characterSprite;

    }
}

entt::entity BrawlerClient::BuildTemp(entt::registry& registry, Sel::Vector2f position, bool bFlip, int skinId)
{
    auto entity = registry.create();

    registry.emplace<TemporaryEntityComponent>(entity, 2.0f);

    registry.emplace<OneShotAnimation>(entity, false, "death", 1.f);

    // Load Ressources
    Sel::ResourceManager& resourceManager = Sel::ResourceManager::Instance();
    std::shared_ptr<Sel::Sprite> characterSprite = std::make_shared<Sel::Sprite>(BuildSpriteStatic(128.f, resourceManager, skinId));
    std::shared_ptr<Sel::Spritesheet> characterSpritesheet = resourceManager.GetSpritesheet("assets/characters/character.spritesheet");

    // Init Graphics Component
    auto& gfxComponent = registry.emplace<Sel::GraphicsComponent>(entity);
    gfxComponent.renderable = characterSprite;

    // Init Spritesheet Component
    auto& spriteSheetComp = registry.emplace<Sel::SpritesheetComponent>(entity, characterSpritesheet, characterSprite);

    auto& transform = registry.emplace<Sel::Transform>(entity);
    transform.SetPosition(position);
    if (bFlip)
        transform.SetScale({ -1.f, 1.f });
    else
        transform.SetScale({ 1.f, 1.f });

    return entity;
}

Sel::Sprite BrawlerClient::BuildSprite(float size, Sel::ResourceManager& resourceManager, int skinId)
{
    Sel::Sprite brawlerSprite(resourceManager.GetTexture("assets/characters/full_character" + std::to_string(skinId) + ".png"));

    brawlerSprite.Resize(size, size);
    brawlerSprite.SetOrigin({ 0.5f, 0.5f });

    return brawlerSprite;
}

Sel::Sprite BrawlerClient::BuildSpriteStatic(float size, Sel::ResourceManager& resourceManager, int skinId)
{
    Sel::Sprite brawlerSprite(resourceManager.GetTexture("assets/characters/full_character" + std::to_string(skinId) + ".png"));

    brawlerSprite.Resize(size, size);
    brawlerSprite.SetOrigin({ 0.5f, 0.5f });

    return brawlerSprite;
}
