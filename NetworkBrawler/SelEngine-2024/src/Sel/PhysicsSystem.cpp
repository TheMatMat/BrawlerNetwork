#include <Sel/PhysicsSystem.hpp>
#include <Sel/RigidBodyComponent.hpp>
#include <Sel/Transform.hpp>
#include <entt/entt.hpp>


namespace Sel
{
	PhysicsSystem::PhysicsSystem(entt::registry& registry) :
	m_registry(registry),
	m_accumulator(0.f),
	m_timestep(1.f / 50.f)
	{
		// Par mesure de simplicité, on utilise un Singleton ici
		// néanmoins, en pratique les singletons sont un peu le "goto de l'architecture".
		// Il serait mieux de permettre d'avoir plusieurs Space
		if (s_instance != nullptr)
			throw std::runtime_error("only one PhysicsSystem can be created");

		s_instance = this;
	}

	PhysicsSystem::~PhysicsSystem()
	{
		// Le PhysicsSystem est détruit avant le registry (qui lui-même détruit les composants RigidBodyComponent, ceux-ci détruisant les cpBody/cpShape)
		// cela pose problème, pour y remédier on va forcer la destruction des composants en les enlevant
		m_registry.clear<RigidBodyComponent>();

		s_instance = nullptr;
	}

	ChipmunkSpace& PhysicsSystem::GetSpace()
	{
		// L'héritage privé interdit à tout autre classe que nous de récupérer une référence sur le parent à partir de nous
		// mais nous pouvons faire une fonction pour l'autoriser
		return *this;
	}

	void PhysicsSystem::Update(float deltaTime)
	{
		m_accumulator += deltaTime;
		while (m_accumulator >= m_timestep)
		{
			Step(m_timestep);
			m_accumulator -= m_timestep;
		}

		auto view = m_registry.view<Transform, RigidBodyComponent>();
		for (entt::entity entity : view)
		{
			Transform& entityTransform = view.get<Transform>(entity);
			RigidBodyComponent& entityBody = view.get<RigidBodyComponent>(entity);

			entityTransform.SetPosition(entityBody.GetPosition());
			entityTransform.SetRotation(entityBody.GetRotation());
		}
	}

	PhysicsSystem* PhysicsSystem::Instance()
	{
		return s_instance;
	}

	PhysicsSystem* PhysicsSystem::s_instance = nullptr;
}