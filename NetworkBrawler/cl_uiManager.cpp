#include "cl_uiManager.h"

UIManager::UIManager(entt::registry& registry) :
	m_registry(registry)
{
}

void UIManager::SetInPos(UIElement& element)
{
    // Check if the entity is valid and exists in the registry
    if (m_registry.valid(element.elementEntity) && m_registry.try_get<Sel::Transform>(element.elementEntity))
    {
        // Retrieve the transform component
        auto& transform = m_registry.get<Sel::Transform>(element.elementEntity);

        // Set the position to the inPos
        transform.SetPosition(element.inPos);
    }
}

void UIManager::DeleteElement(UIElement& element)
{

}

void UIManager::BringIn(UIElement& element)
{
}

void UIManager::BringOut(UIElement& element)
{
}
