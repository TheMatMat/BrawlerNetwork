#include <Sel/VelocityComponent.hpp>
#include <Sel/JsonSerializer.hpp>
#include <Sel/Math.hpp>
#include <entt/entt.hpp>
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace Sel
{
	void VelocityComponent::PopulateInspector(WorldEditor& /*worldEditor*/)
	{
		float velArray[2] = { linearVel.x, linearVel.y };
		if (ImGui::InputFloat2("Linear vel", velArray))
			linearVel = Sel::Vector2f({ velArray[0], velArray[1] });

		float angular = angularVel * Deg2Rad;
		if (ImGui::SliderAngle("Rotation", &angular))
			angularVel = angular * Rad2Deg;
	}

	nlohmann::json VelocityComponent::Serialize(const entt::handle entity) const
	{
		nlohmann::json doc;
		doc["LinearVel"] = linearVel;
		doc["AngularVel"] = angularVel;

		return doc;
	}

	void VelocityComponent::Unserialize(entt::handle entity, const nlohmann::json& doc)
	{
		auto& vel = entity.emplace<VelocityComponent>();
		vel.angularVel = doc.value("AngularVel", 0.f);
		vel.linearVel = doc.value("LinearVel", Vector2f(0, 0));
	}
}