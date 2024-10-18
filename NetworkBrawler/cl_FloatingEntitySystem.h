#pragma once

#include "entt/entt.hpp"
#include "Sel/Vector2.hpp"
#include "Sel/Transform.hpp"

class FloatingEntitySystem
{
public:
	FloatingEntitySystem(entt::registry* registry);
	~FloatingEntitySystem();

	void AddFloatingEntity(entt::entity anchor, entt::entity floating, Sel::Vector2f offset);

	void Update();

private:
	struct FloatingEntityData
	{
		entt::entity anchor;
		entt::entity floating;
		Sel::Vector2f offset;
	};

	entt::registry* m_registry;
	std::vector<FloatingEntityData> m_floatingEntities;
};