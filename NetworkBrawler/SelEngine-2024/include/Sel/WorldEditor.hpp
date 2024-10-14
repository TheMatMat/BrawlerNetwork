#pragma once

#include <Sel/Export.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace Sel
{
	class ComponentRegistry;
	class Transform;
	class Window;

	class SEL_ENGINE_API WorldEditor
	{
		public:
			WorldEditor(Window& window, entt::registry& registry, const ComponentRegistry& componentRegistry);
			WorldEditor(const WorldEditor&) = delete;
			WorldEditor(WorldEditor&&) = delete;
			~WorldEditor() = default;

			void Render();

			entt::registry& GetRegistry();
			const entt::registry& GetRegistry() const;

			bool IsPaused() const;

			WorldEditor& operator=(const WorldEditor&) = delete;
			WorldEditor& operator=(WorldEditor&&) = delete;

		private:
			void CenterCameraOnEntity(entt::entity entity);
			void DisplayEntityList();
			void DisplayEntityListNode(entt::entity entity);
			bool EntityInspector(entt::entity entity);
			entt::entity GetCameraEntity();
			entt::entity GetEntity(Transform* transform);
			void LoadScene();
			void SaveScene();

			std::vector<entt::entity> m_inspectedEntities;
			entt::registry& m_registry;
			const ComponentRegistry& m_componentRegistry;
			Window& m_window;
			bool m_isPaused;
			std::string m_scenePath;
	};
}