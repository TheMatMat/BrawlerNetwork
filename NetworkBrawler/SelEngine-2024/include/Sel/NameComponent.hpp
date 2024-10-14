#pragma once

#include <Sel/Export.hpp>
#include <entt/fwd.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Sel
{
	class WorldEditor;

	// Petit composant pour nommer une entité (pour l'inspecteur)
	struct SEL_ENGINE_API NameComponent
	{
		explicit NameComponent(std::string Name);

		void PopulateInspector(WorldEditor& worldEditor);
		nlohmann::json Serialize(const entt::handle entity) const;
		static void Unserialize(entt::handle entity, const nlohmann::json& doc);

		std::string name;
	};
}