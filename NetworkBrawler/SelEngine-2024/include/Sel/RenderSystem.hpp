#pragma once

#include <Sel/Export.hpp>
#include <entt/fwd.hpp> //< header spécial qui fait des déclarations anticipées des classes de la lib

namespace Sel
{
	class Renderer;

	class SEL_ENGINE_API RenderSystem
	{
		public:
			RenderSystem(Renderer& renderer, entt::registry& registry);

			void Update(float deltaTime);

		private:
			Renderer& m_renderer;
			entt::registry& m_registry;
	};
}
