#pragma once

#include "sh_brawler.h"
#include <entt/entt.hpp>
#include <Sel/Sprite.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/SpritesheetComponent.hpp>
#include <Sel/Transform.hpp>
#include <Sel/Renderer.hpp>
#include <Sel/Font.hpp>

class BrawlerClient : public Brawler
{
public:
    BrawlerClient(entt::registry& registry, Sel::Renderer& renderer);
    BrawlerClient(entt::registry& registry, const Sel::Vector2f& position, float rotation, float scale, const Sel::Vector2f& linearVelocity);
    ~BrawlerClient() = default;

    static entt::entity BuildTemp(entt::registry& registry, Sel::Vector2f position, bool bFlip = false);

protected:
   Sel::Sprite BuildSprite(float size, Sel::ResourceManager& resourceManager);
   static Sel::Sprite BuildSpriteStatic(float size, Sel::ResourceManager& resourceManager);
};