#pragma once

#include <entt/entt.hpp>

struct TemporaryEntityComponent
{
	float lifeTime;
};

class TemporaryEntitySystem
{
public:
	TemporaryEntitySystem() = delete;
	TemporaryEntitySystem(entt::registry& registry);

	void Update(float deltaTime);

private:
	entt::registry& m_registry;
};