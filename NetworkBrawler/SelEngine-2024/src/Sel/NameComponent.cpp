#include <Sel/NameComponent.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h> //< ImGui::InputText avec std::string
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace Sel
{
	NameComponent::NameComponent(std::string Name) :
	name(std::move(Name))
	{
	}

	void NameComponent::PopulateInspector(WorldEditor& /*worldEditor*/)
	{
		ImGui::InputText("Name", &name);
	}

	nlohmann::json NameComponent::Serialize(const entt::handle entity) const
	{
		nlohmann::json doc;
		doc["Name"] = name;

		return doc;
	}

	void NameComponent::Unserialize(entt::handle entity, const nlohmann::json& doc)
	{
		entity.emplace<NameComponent>(doc.value("Name", ""));
	}
}
