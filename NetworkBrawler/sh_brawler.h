#pragma once

#include <entt/entt.hpp>
#include "sh_protocol.h"
#include "sh_constants.h"
#include "Sel/Sprite.hpp"

struct PlayerInputs;

class Brawler
{
public:
	Brawler() = default;
	Brawler(entt::registry& registry);
	Brawler(entt::registry& registry, const Sel::Vector2f& position, float rotation, float scale, const Sel::Vector2f& linearVelocity);
	virtual ~Brawler() = default;

	const Sel::Vector2f& GetPosition() const;
	const Sel::Vector2f& GetVelocity() const;
	const entt::handle& GetHandle();
	std::uint32_t GetId();

	void ApplyInputs(const PlayerInputs& inputs);

protected:
	entt::handle m_handle;
	Sel::Vector2f m_position;
	Sel::Vector2f m_linearVelocity;

	float m_speed;
};