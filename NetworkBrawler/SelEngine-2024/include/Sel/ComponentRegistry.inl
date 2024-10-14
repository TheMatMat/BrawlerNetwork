#include <imgui.h>
#include <type_traits>

namespace Sel
{
	template<typename T>
	std::function<void(entt::handle)> ComponentRegistry::BuildAddComponent()
	{
		return [](entt::handle entity)
		{
			return entity.emplace<T>();
		};
	}

	template<typename T>
	std::function<bool(entt::handle)> ComponentRegistry::BuildHasComponent()
	{
		return [](entt::handle entity)
		{
			return entity.any_of<T>();
		};
	}

	template<typename T>
	std::function<void(entt::handle)> ComponentRegistry::BuildRemoveComponent()
	{
		return [](entt::handle entity)
		{
			return entity.erase<T>();
		};
	}

	template<typename T>
	auto ComponentRegistry::BuildInspect() -> std::function<void(WorldEditor&, entt::handle)>
	{
		return [](WorldEditor& worldEditor, entt::handle entity)
		{
			entity.get<T>().PopulateInspector(worldEditor);
		};
	}

	template<typename T>
	std::function<nlohmann::json(entt::handle)> ComponentRegistry::BuildSerialize()
	{
		return [](entt::handle entity)
		{
			if constexpr (!std::is_empty_v<T>)
				return entity.get<T>().Serialize(entity);
			else
				return nlohmann::json::object();
		};
	}

	template<typename T>
	std::function<void(entt::handle, const nlohmann::json&)> ComponentRegistry::BuildUnserialize()
	{
		return [](entt::handle entity, const nlohmann::json& doc)
		{
			if constexpr (!std::is_empty_v<T>)
				T::Unserialize(entity, doc);
			else
				entity.emplace<T>();
		};
	}

}
