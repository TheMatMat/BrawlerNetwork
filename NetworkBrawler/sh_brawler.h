#pragma once

#include <entt/entt.hpp>
#include "sh_protocol.h"



class Brawler
{
public:
	Brawler() = default;
	Brawler(entt::registry& registry, const Sel::Vector2f& position);
	virtual ~Brawler() = default;

	const Sel::Vector2f& GetPosition() const;
	const Sel::Vector2f& GetVelocity() const;

protected:
	entt::handle m_handle;
	Sel::Vector2f m_position;
	Sel::Vector2f m_linearVelocity;
};