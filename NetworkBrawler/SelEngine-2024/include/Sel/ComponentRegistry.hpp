#pragma once

#include <Sel/Export.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <functional>
#include <string>

namespace Sel
{
	class WorldEditor;

	// Pour permettre l'ajout de composants utilisateurs et avoir une liste des différents
	// composants que nous pouvons sérialiser/inspecter sans l'écrire en dur, nous pouvons passer
	// par un petit registre (contenant une entrée par type de composant éditable)
	class SEL_ENGINE_API ComponentRegistry
	{
		public:
			struct Entry
			{
				std::string id;
				std::string label;
				std::function<void(entt::handle)> addComponent;
				std::function<bool(entt::handle)> hasComponent;
				std::function<void(entt::handle)> removeComponent;
				std::function<void(WorldEditor&, entt::handle)> inspect;
				std::function<nlohmann::json(entt::handle)> serialize;
				std::function<void(entt::handle, const nlohmann::json&)> unserialize;
			};

			ComponentRegistry();
			ComponentRegistry(const ComponentRegistry&) = delete;
			ComponentRegistry(ComponentRegistry&&) = delete;
			~ComponentRegistry() = default;

			void ForEachComponent(const std::function<void(const Entry&)>& callback) const;

			void Register(Entry&& data);

			ComponentRegistry& operator=(const ComponentRegistry&) = delete;
			ComponentRegistry& operator=(ComponentRegistry&&) = delete;

			template<typename T> static std::function<void(entt::handle)> BuildAddComponent();
			template<typename T> static std::function<bool(entt::handle)> BuildHasComponent();
			template<typename T> static std::function<void(entt::handle)> BuildRemoveComponent();
			template<typename T> static std::function<void(WorldEditor&, entt::handle)> BuildInspect();
			template<typename T> static std::function<nlohmann::json(entt::handle)> BuildSerialize();
			template<typename T> static std::function<void(entt::handle, const nlohmann::json&)> BuildUnserialize();

		private:
			void RegisterEngineComponents();

			std::vector<Entry> m_componentTypes;
	};
}

#include <Sel/ComponentRegistry.inl>