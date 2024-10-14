#include <Sel/WorldEditor.hpp>
#include <Sel/CameraComponent.hpp>
#include <Sel/ComponentRegistry.hpp>
#include <Sel/NameComponent.hpp>
#include <Sel/Transform.hpp>
#include <Sel/Window.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <fstream>

namespace Sel
{
	constexpr unsigned int FileVersion = 1;

	WorldEditor::WorldEditor(Window& window, entt::registry& registry, const ComponentRegistry& componentRegistry) :
	m_registry(registry),
	m_componentRegistry(componentRegistry),
	m_window(window),
	m_isPaused(false)
	{
	}

	void WorldEditor::Render()
	{
		ImGui::Begin("World editor");
		{
			if (ImGui::Button((m_isPaused) ? "Unpause" : "Pause"))
				m_isPaused = !m_isPaused;

			if (ImGui::Button("Create entity"))
			{
				entt::entity entity = m_registry.create();
				m_inspectedEntities.push_back(entity);
			}

			if (ImGui::TreeNode("Entity list"))
			{
				DisplayEntityList();
				ImGui::TreePop();
			}

			ImGui::InputText("Path", &m_scenePath);

			if (ImGui::Button("Save scene"))
				SaveScene();

			ImGui::SameLine();

			if (ImGui::Button("Load scene"))
				LoadScene();
		}
		ImGui::End();

		// Pattern permettant d'itérer sur un vector tout en supprimant conditionnellement des entités
		for (auto it = m_inspectedEntities.begin(); it != m_inspectedEntities.end();)
		{
			if (!m_registry.valid(*it) || !EntityInspector(*it))
				it = m_inspectedEntities.erase(it);
			else
				++it;
		}
	}

	entt::registry& WorldEditor::GetRegistry()
	{
		return m_registry;
	}

	const entt::registry& WorldEditor::GetRegistry() const
	{
		return m_registry;
	}

	bool WorldEditor::IsPaused() const
	{
		return m_isPaused;
	}

	void WorldEditor::CenterCameraOnEntity(entt::entity entity)
	{
		Sel::Transform* targetTransform = m_registry.try_get<Sel::Transform>(entity);
		if (!targetTransform)
		{
			fmt::print(fg(fmt::color::red), "entity has no transform\n");
			return;
		}

		entt::entity camera = GetCameraEntity();
		if (camera == entt::null)
		{
			fmt::print(fg(fmt::color::red), "no camera entity\n");
			return;
		}

		Sel::Transform& cameraTransform = m_registry.get<Sel::Transform>(camera);

		// On part du principe que la caméra elle-même n'est pas attachée à une autre entité (notre SetPosition étant local)
		Vector2i windowSize = m_window.GetSize();

		// Petit ajustement de la position pour centrer la caméra sur l'entité
		cameraTransform.SetPosition(targetTransform->GetGlobalPosition() - Vector2f(windowSize.x, windowSize.y) * 0.5f);
	}

	void WorldEditor::DisplayEntityList()
	{
		for (auto [entity] : m_registry.storage<entt::entity>().each())
		{
			// On vérifie que l'entité n'est pas parentée à une autre (on parle d'entité "root")
			if (Sel::Transform* transform = m_registry.try_get<Sel::Transform>(entity))
			{
				if (transform->GetParent())
					continue;
			}

			DisplayEntityListNode(entity);
		}
	}

	void WorldEditor::DisplayEntityListNode(entt::entity entity)
	{
		std::string entityName = fmt::format("Entity #{}", static_cast<std::uint32_t>(entity)); //< On se rappelle qu'un entt::entity est un entier
		if (Sel::NameComponent* nameComponent = m_registry.try_get<Sel::NameComponent>(entity))
		{
			if (!nameComponent->name.empty())
				entityName = fmt::format("{} - {}", entityName, nameComponent->name);
		}

		auto HandleContextMenu = [&]
		{
			// Ces fonctions agissent sur le dernier item donné à imgui, il est donc important de pouvoir les appeler au bon moment
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				// On vient de double-cliquer sur l'entité
				auto it = std::find(m_inspectedEntities.begin(), m_inspectedEntities.end(), entity);

				// Est-ce que l'entité est déjà inspectée ?
				if (it != m_inspectedEntities.end())
					m_inspectedEntities.erase(it); // On l'enlève de la liste
				else
					m_inspectedEntities.push_back(entity); // Sinon on la rajoute
			}

			if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::MenuItem("Go to"))
					CenterCameraOnEntity(entity);

				if (ImGui::MenuItem("Delete"))
					m_registry.destroy(entity);

				ImGui::EndPopup();
			}
		};

		// Affichage de la hiérarchie
		// (il est 2h23 du matin donc excusez la duplication de code)
		if (Sel::Transform* transform = m_registry.try_get<Sel::Transform>(entity))
		{
			const std::vector<Transform*>& childTransforms = transform->GetChildren();

			ImGuiTreeNodeFlags nodeFlags = (childTransforms.empty()) ? ImGuiTreeNodeFlags_Leaf : 0;
			if (ImGui::TreeNodeEx(entityName.c_str(), nodeFlags))
			{
				HandleContextMenu();

				for (Transform* childTransform : childTransforms)
				{
					entt::entity childEntity = GetEntity(childTransform);
					if (childEntity != entt::null)
						DisplayEntityListNode(childEntity);
				}

				ImGui::TreePop();
			}
			else
				HandleContextMenu();
		}
		else
		{
			if (ImGui::TreeNodeEx(entityName.c_str(), ImGuiTreeNodeFlags_Leaf))
			{
				HandleContextMenu();
				ImGui::TreePop();
			}
			else
				HandleContextMenu();
		}
	}

	bool WorldEditor::EntityInspector(entt::entity entity)
	{
		std::string windowName = fmt::format("Entity inspector #{}", static_cast<std::uint32_t>(entity));

		ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_Appearing);

		bool stayOpen = true;
		ImGui::Begin(windowName.c_str(), &stayOpen);
		{
			std::string deleteModalId = fmt::format("Delete {}", static_cast<std::uint32_t>(entity));
			if (ImGui::Button("Delete entity"))
				ImGui::OpenPopup(deleteModalId.c_str());

			if (ImGui::BeginPopupModal(deleteModalId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::Text("Are you sure you want to delete this entity?\nThis operation cannot be undone!\n\n");
				ImGui::Separator();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					m_registry.destroy(entity);
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}

			entt::handle entityHandle(m_registry, entity);
			bool canAddComponent = false;
			m_componentRegistry.ForEachComponent([&](const ComponentRegistry::Entry& entry)
			{
				assert(entry.hasComponent);
				if (entry.hasComponent(entityHandle))
				{
					if (!ImGui::TreeNode(entry.label.c_str()))
						return;

					if (entry.inspect)
						entry.inspect(*this, entityHandle);

					if (entry.removeComponent && ImGui::Button(fmt::format("Remove {} component", entry.label).c_str()))
						entry.removeComponent(entityHandle);

					ImGui::TreePop();
				}
				else if (entry.addComponent)
					canAddComponent = true; //< on peut encore ajouter au moins un composant
			});

			if (canAddComponent)
			{
				if (ImGui::Button("Add component..."))
					ImGui::OpenPopup("AddComponent");

				if (ImGui::BeginPopup("AddComponent"))
				{
					bool closePopup = false;
					if (ImGui::BeginCombo("Add component", "Choose..."))
					{
						m_componentRegistry.ForEachComponent([&](const ComponentRegistry::Entry& entry)
						{
							assert(entry.hasComponent);
							if (entry.hasComponent(entityHandle))
								return;

							if (entry.addComponent && ImGui::Selectable(entry.label.c_str()))
							{
								entry.addComponent(entityHandle);
								closePopup = true;
							}
						});

						ImGui::EndCombo();
					}

					if (closePopup)
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();

		return stayOpen;
	}

	entt::entity WorldEditor::GetCameraEntity()
	{
		for (entt::entity camera : m_registry.view<Transform, CameraComponent>())
			return camera;

		return entt::null;
	}

	entt::entity WorldEditor::GetEntity(Transform* transform)
	{
		// On recherche l'entité possédant le composant Transform
		// on aurait pu faire différemment (stocker l'entité propriétaire dans le Transform, faire une map)
		// mais dans notre cas ça ne posera pas de problème de performance (et il est 2h25 du matin)

		for (auto&& [entity, entityTransform] : m_registry.view<Transform>().each())
		{
			if (&entityTransform == transform) //< on compare les pointeurs
				return entity;
		}

		return entt::null; //< ne devrait pas arriver
	}

	void WorldEditor::LoadScene()
	{
		std::ifstream inputFile(m_scenePath);
		if (!inputFile)
		{
			fmt::print(fg(fmt::color::red), "failed to open {}\n", m_scenePath);
			return;
		}

		nlohmann::json sceneDoc;
		try
		{
			// On a pas très envie qu'une vilaine petite erreur de parsing (causée - à tout hasard - par un conflit Git)
			// crashe l'éditeur au lancement
			sceneDoc = nlohmann::json::parse(inputFile);
		}
		catch (const std::exception& e)
		{
			fmt::print(fg(fmt::color::red), "failed to parse {}: {}\n", m_scenePath, e.what());
			return;
		}

		unsigned int version = sceneDoc["Version"];
		if (version > FileVersion)
		{
			fmt::print(fg(fmt::color::red), "{} has an unknown file version {}\n", m_scenePath, version);
			return;
		}

		// On réinitialise le monde pour créer les entités du document
		m_registry = entt::registry{};

		std::vector<entt::entity> indexToEntity;
		for (const nlohmann::json& entityDoc : sceneDoc["Entities"])
		{
			// Création de l'entité
			entt::handle entityHandle(m_registry, m_registry.create());
			indexToEntity.push_back(entityHandle);

			m_componentRegistry.ForEachComponent([&](const ComponentRegistry::Entry& entry)
			{
				if (!entry.unserialize)
					return;

				auto it = entityDoc.find(entry.id);
				if (it != entityDoc.end())
					entry.unserialize(entityHandle, it.value());
			});
		}

		for (const nlohmann::json& hierarchyDoc : sceneDoc["Hierarchies"])
		{
			unsigned int parentId = hierarchyDoc["Parent"];
			unsigned int childId = hierarchyDoc["Child"];

			Transform& parentTransform = m_registry.get<Transform>(indexToEntity[parentId]);
			Transform& childTransform = m_registry.get<Transform>(indexToEntity[childId]);
			childTransform.SetParent(&parentTransform);
		}

	}

	void WorldEditor::SaveScene()
	{
		std::ofstream fileStream(m_scenePath);
		if (!fileStream)
		{
			fmt::print(fg(fmt::color::red), "failed to open {}\n", m_scenePath);
			return;
		}

		struct Hierarchy
		{
			entt::entity parent;
			entt::entity child;
		};

		std::vector<Hierarchy> hierarchies;
		std::unordered_map<entt::entity, unsigned int> entityToIndex;

		// On sauvegarde tous les composants de toutes les entités
		nlohmann::json entityArray;
		for (auto [entity] : m_registry.storage<entt::entity>().each())
		{
			entt::handle entityHandle(m_registry, entity);
			
			nlohmann::json entityDoc;
			m_componentRegistry.ForEachComponent([&](const ComponentRegistry::Entry& entry)
			{
				assert(entry.hasComponent);
				if (entry.hasComponent(entityHandle) && entry.serialize)
					entityDoc[entry.id] = entry.serialize(entityHandle);
			});

			if (Transform* transform = entityHandle.try_get<Transform>())
			{
				if (Transform* parentTransform = transform->GetParent())
				{
					entt::entity parentEntity = GetEntity(parentTransform); 
					hierarchies.push_back({ parentEntity, entity });
				}
			}

			entityToIndex[entity] = entityArray.size();
			entityArray.push_back(std::move(entityDoc));
		}

		nlohmann::ordered_json sceneDoc;
		sceneDoc["Version"] = FileVersion;
		sceneDoc["Entities"] = std::move(entityArray);

		nlohmann::json hierarchiesDoc;
		for (auto&& [parent, child] : hierarchies)
		{
			nlohmann::json hierarchyDoc;

			hierarchyDoc["Parent"] = entityToIndex[parent];
			hierarchyDoc["Child"] = entityToIndex[child];

			hierarchiesDoc.push_back(std::move(hierarchyDoc));
		}
		sceneDoc["Hierarchies"] = std::move(hierarchiesDoc);

		fileStream << sceneDoc.dump(1, '\t');

		fmt::print(fg(fmt::color::green), "scene saved to {}\n", m_scenePath);
	}
}
