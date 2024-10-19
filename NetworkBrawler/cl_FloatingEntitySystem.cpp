#include "cl_FloatingEntitySystem.h"

FloatingEntitySystem::FloatingEntitySystem(entt::registry* registry)
	: m_registry(registry)
{
}

FloatingEntitySystem::~FloatingEntitySystem()
{
}

void FloatingEntitySystem::AddFloatingEntity(entt::entity anchor, entt::entity floating, Sel::Vector2f offset)
{
	FloatingEntityData floatingEntity;
	floatingEntity.anchor = anchor;
	floatingEntity.floating = floating;
	floatingEntity.offset = offset;

	m_floatingEntities.push_back(floatingEntity);
}

void FloatingEntitySystem::Update()
{
    std::vector<size_t> entitiesToRemove;

    for (size_t i = 0; i < m_floatingEntities.size(); ++i)
    {
        auto& floatingEntity = m_floatingEntities[i];
        auto anchorTransform = m_registry->try_get<Sel::Transform>(floatingEntity.anchor);
        auto floatingTransform = m_registry->try_get<Sel::Transform>(floatingEntity.floating);

        if (anchorTransform && floatingTransform)
        {
            // Update floating entity's position relative to anchor
            floatingTransform->SetPosition(anchorTransform->GetGlobalPosition() + floatingEntity.offset);
            continue;
        }

        // If anchor is missing, mark floating entity for removal
        if (!anchorTransform && floatingTransform)
        {
            m_registry->destroy(floatingEntity.floating);
            entitiesToRemove.push_back(i);
        }

        if(!floatingTransform)
            entitiesToRemove.push_back(i);
    }

    // Remove entries with no anchors
    for (auto it = entitiesToRemove.rbegin(); it != entitiesToRemove.rend(); ++it)
    {
        m_floatingEntities.erase(m_floatingEntities.begin() + *it);
    }
}
