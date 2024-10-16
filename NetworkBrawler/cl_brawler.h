#pragma once

#include "sh_brawler.h"
#include <entt/entt.hpp>
#include <Sel/Sprite.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/GraphicsComponent.hpp>

class BrawlerClient : public Brawler
{
public:
    BrawlerClient(entt::registry& registry);
    BrawlerClient(entt::registry& registry, const Sel::Vector2f& position, float rotation, float scale, const Sel::Vector2f& linearVelocity);
    ~BrawlerClient() = default;

protected:
    Sel::Sprite BuildSprite(float size);
};