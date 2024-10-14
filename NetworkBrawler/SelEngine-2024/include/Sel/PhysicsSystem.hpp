#pragma once

#include <Sel/Export.hpp>
#include <Sel/ChipmunkSpace.hpp>
#include <entt/fwd.hpp>

namespace Sel
{
	class SEL_ENGINE_API PhysicsSystem : ChipmunkSpace //< héritage privé, les méthodes ne sont pas accessibles publiquement
	{
		public:
			PhysicsSystem(entt::registry& registry);
			PhysicsSystem(const PhysicsSystem&) = delete;
			PhysicsSystem(PhysicsSystem&&) = default;
			~PhysicsSystem();

			ChipmunkSpace& GetSpace();

			// Cette syntaxe permet d'exposer publiquement des méthodes cachées du parent
			using ChipmunkSpace::DebugDraw;
			using ChipmunkSpace::SetDamping;
			using ChipmunkSpace::SetGravity;

			void Update(float deltaTime);

			PhysicsSystem& operator=(const PhysicsSystem&) = delete;
			PhysicsSystem& operator=(PhysicsSystem&&) = delete;

			static PhysicsSystem* Instance();

		private:
			entt::registry& m_registry;
			float m_accumulator;
			float m_timestep;

			static PhysicsSystem* s_instance;
	};
}
