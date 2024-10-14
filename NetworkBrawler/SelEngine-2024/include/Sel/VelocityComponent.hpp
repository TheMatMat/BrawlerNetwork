#pragma once

#include <Sel/Vector2.hpp>
#include <entt/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Sel
{
	class WorldEditor;

	struct VelocityComponent
	{
		Vector2f linearVel = Vector2f(0.f, 0.f);
		float angularVel = 0.f;

		void PopulateInspector(WorldEditor& worldEditor);
		nlohmann::json Serialize(const entt::handle entity) const;
		static void Unserialize(entt::handle entity, const nlohmann::json& doc);
	};
}