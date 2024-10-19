#pragma once

#include <entt/entt.hpp>
#include <Sel/Vector2.hpp>
#include <Sel/Transform.hpp>

class UIManager
{
public:
	UIManager() = delete;
	UIManager(entt::registry& registry);

	struct UIElement
	{
		entt::entity elementEntity = entt::null;
		Sel::Vector2f inPos = { 0.f, 0.f };

		UIElement& operator=(const std::tuple<entt::registry*, entt::entity, Sel::Vector2f>& newRegistryEntityAndPos)
		{
			entt::registry* registry = std::get<0>(newRegistryEntityAndPos);
			entt::entity newEntity = std::get<1>(newRegistryEntityAndPos);
			Sel::Vector2f newInPos = std::get<2>(newRegistryEntityAndPos);

			// Remove the old entity from the registry if valid
			if (registry && elementEntity != entt::null && registry->valid(elementEntity))
			{
				registry->destroy(elementEntity);
			}

			// Assign the new registry, entity, and position
			elementEntity = newEntity;
			inPos = newInPos;

			return *this;
		}
	};

	void SetInPos(UIElement& element);
	void DeleteElement(UIElement& element);
	void BringIn(UIElement& element);
	void BringOut(UIElement& element);

	// All UI elements
	UIElement getReadyText;
	UIElement isReadyText;

private:
	entt::registry& m_registry;
};