#include "sh_temporaryEntitySystem.h"

TemporaryEntitySystem::TemporaryEntitySystem(entt::registry& registry)
	: m_registry(registry)
{

}

void TemporaryEntitySystem::Update(float deltaTime)
{
	auto view = m_registry.view<TemporaryEntityComponent>();
	for (auto&& [entity, temporaryComp] : view.each())
	{
		temporaryComp.lifeTime -= deltaTime;
		if (temporaryComp.lifeTime <= 0.f)
			m_registry.destroy(entity);
	}
}
