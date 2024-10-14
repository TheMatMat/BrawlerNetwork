#pragma once

#include <Sel/Export.hpp>
#include <entt/fwd.hpp> //< header spécial qui fait des déclarations anticipées des classes de la lib

namespace Sel
{
	class SEL_ENGINE_API AnimationSystem
	{
		public:
			AnimationSystem(entt::registry& registry);

			void Update(float deltaTime);

		private:
			entt::registry& m_registry;
	};
}