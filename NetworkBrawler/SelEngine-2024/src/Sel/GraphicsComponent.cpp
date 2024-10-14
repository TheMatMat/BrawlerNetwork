#include <Sel/GraphicsComponent.hpp>
#include <Sel/Model.hpp>
#include <Sel/Renderable.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/Sprite.hpp>
#include <entt/entt.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>

namespace Sel
{
	void GraphicsComponent::PopulateInspector(WorldEditor& worldEditor)
	{
		if (renderable)
		{
			if (ImGui::Button("Clear"))
				renderable = nullptr;
			else
				renderable->PopulateInspector(worldEditor);
		}
		else
		{
			ImGui::InputText("Path", &filepath);

			if (ImGui::Button("Create model"))
				renderable = ResourceManager::Instance().GetModel(filepath);
			else if (ImGui::Button("Create sprite"))
				renderable = std::make_shared<Sprite>(ResourceManager::Instance().GetTexture(filepath));
		}
	}

	nlohmann::json GraphicsComponent::Serialize(const entt::handle entity) const
	{
		nlohmann::json doc;
		if (renderable)
			doc["Renderable"] = renderable->Serialize();

		return doc;
	}

	void GraphicsComponent::Unserialize(entt::handle entity, const nlohmann::json& doc)
	{
		auto& gfxComponent = entity.emplace<GraphicsComponent>();

		if (auto it = doc.find("Renderable"); it != doc.end())
		{
			const nlohmann::json& renderableDoc = it.value();

			// On identifie le type de renderable via la valeur "Type" que Model et Sprite écrivent
			// Ce n'est pas très scalable, on devrait faire une factory (une map entre un nom et une fonction instanciant le renderable)
			std::string renderableType = renderableDoc["Type"];
			if (renderableType == "Model")
				gfxComponent.renderable = Model::Unserialize(renderableDoc);
			else if (renderableType == "Sprite")
				gfxComponent.renderable = Sprite::Unserialize(renderableDoc);
			else
				fmt::print(fg(fmt::color::red), "unknown renderable \"{}\"\n", renderableType);
		}
	}
}
