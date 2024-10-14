#include <Sel/Transform.hpp>
#include <Sel/JsonSerializer.hpp>
#include <Sel/NameComponent.hpp>
#include <Sel/WorldEditor.hpp>
#include <entt/entt.hpp>
#include <fmt/format.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cassert>

namespace Sel
{
	Transform::Transform() :
	m_parent(nullptr),
	m_position(0.f, 0.f),
	m_rotation(0.f),
	m_scale(1.f, 1.f)
	{
	}

	Transform::Transform(const Transform& transform) :
	m_parent(nullptr),
	m_position(transform.m_position),
	m_rotation(transform.m_rotation),
	m_scale(transform.m_scale)
	{
		SetParent(transform.m_parent);
	}

	Transform::Transform(Transform&& transform) noexcept :
	m_children(std::move(transform.m_children)),
	m_parent(nullptr),
	m_position(transform.m_position),
	m_rotation(transform.m_rotation),
	m_scale(transform.m_scale)
	{
		SetParent(transform.m_parent);
		for (Transform* child : m_children)
			child->m_parent = this;
	}

	Transform::~Transform()
	{
		if (m_parent)
			m_parent->DetachChild(this);

		for (Transform* child : m_children)
			child->m_parent = nullptr;
	}

	const std::vector<Transform*>& Transform::GetChildren() const
	{
		return m_children;
	}

	Transform* Transform::GetParent() const
	{
		return m_parent;
	}

	Vector2f Transform::GetGlobalPosition() const
	{
		if (!m_parent)
			return m_position;

		return m_parent->TransformPoint(m_position);
	}

	float Transform::GetGlobalRotation() const
	{
		if (!m_parent)
			return m_rotation;

		return m_parent->GetGlobalRotation() + m_rotation;
	}

	Vector2f Transform::GetGlobalScale() const
	{
		if (!m_parent)
			return m_scale;

		return m_parent->GetGlobalScale() * m_scale;
	}

	const Vector2f& Transform::GetPosition() const
	{
		return m_position;
	}

	float Transform::GetRotation() const
	{
		return m_rotation;
	}

	const Vector2f& Transform::GetScale() const
	{
		return m_scale;
	}

	Matrix3f Transform::GetTransformMatrix() const
	{
		// Construction d'une matrice appliquant le scale puis la rotation puis la translation (l'ordre des opérands est inversé dû à la convention mathématique)
		Matrix3f transformMatrix = Matrix3f::Translate(m_position) * Matrix3f::Rotate(m_rotation) * Matrix3f::Scale(m_scale);

		// Application de la transformation du parent avant la notre (encore une fois, l'ordre des opérands est inversé par rapport à l'ordre naturel)
		if (m_parent)
			transformMatrix = m_parent->GetTransformMatrix() * transformMatrix;

		return transformMatrix;
	}

	void Transform::PopulateInspector(WorldEditor& worldEditor)
	{
		float posArray[2] = { m_position.x, m_position.y };
		if (ImGui::InputFloat2("Position", posArray))
			SetPosition({ posArray[0], posArray[1] });

		float rot = m_rotation * Sel::Deg2Rad;
		if (ImGui::SliderAngle("Rotation", &rot))
			SetRotation(rot * Sel::Rad2Deg);

		float scaleArray[2] = { m_scale.x, m_scale.y };
		if (ImGui::InputFloat2("Scale", scaleArray))
			SetScale({ scaleArray[0], scaleArray[1] });

		if (ImGui::Button("Reparent..."))
			ImGui::OpenPopup("Parent");

		if (ImGui::BeginPopup("Parent"))
		{
			bool closePopup = false;
			if (ImGui::BeginCombo("New parent", "Choose..."))
			{
				if (m_parent)
				{
					if (ImGui::Selectable("Unparent"))
					{
						SetParent(nullptr);
						closePopup = true;
					}
				}

				entt::registry& registry = worldEditor.GetRegistry();
				for (auto&& [entity, transform] : registry.view<Transform>().each())
				{
					if (&transform == this)
						continue;

					std::string entityName = fmt::format("Entity #{}", static_cast<std::uint32_t>(entity)); //< On se rappelle qu'un entt::entity est un entier
					if (const Sel::NameComponent* nameComponent = registry.try_get<Sel::NameComponent>(entity))
					{
						if (!nameComponent->name.empty())
							entityName = fmt::format("{} - {}", entityName, nameComponent->name);
					}

					if (ImGui::Selectable(entityName.c_str()))
					{
						SetParent(&transform);
						closePopup = true;
					}
				}

				ImGui::EndCombo();
			}

			if (closePopup)
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (ImGui::TreeNode("Transform matrix"))
		{
			Matrix3f transformMatrix = GetTransformMatrix();
			ImGui::Text("%f %f %f", transformMatrix(0, 0), transformMatrix(0, 1), transformMatrix(0, 2));
			ImGui::Text("%f %f %f", transformMatrix(1, 0), transformMatrix(1, 1), transformMatrix(1, 2));
			ImGui::Text("%f %f %f", transformMatrix(2, 0), transformMatrix(2, 1), transformMatrix(2, 2));
			ImGui::TreePop();
		}
	}

	void Transform::Rotate(float rotation)
	{
		m_rotation += rotation;
	}

	void Transform::Scale(float scale)
	{
		m_scale *= scale;
	}

	void Transform::Scale(const Vector2f& scale)
	{
		m_scale *= scale;
	}

	nlohmann::json Transform::Serialize(const entt::handle entity) const
	{
		// On sauvegarde les propriétés du transform
		nlohmann::json doc;
		doc["Position"] = m_position;
		doc["Rotation"] = m_rotation;
		doc["Scale"] = m_scale;

		return doc;
	}

	void Transform::SetParent(Transform* parent)
	{
		if (m_parent == parent)
			return;

		if (m_parent)
			m_parent->DetachChild(this);

		m_parent = parent;
		if (m_parent)
			m_parent->AttachChild(this);
	}

	void Transform::SetPosition(const Vector2f& position)
	{
		m_position = position;
	}

	void Transform::SetRotation(float rotation)
	{
		m_rotation = rotation;
	}

	void Transform::SetScale(const Vector2f& scale)
	{
		m_scale = scale;
	}

	void Transform::Translate(const Vector2f& translation)
	{
		m_position += translation;
	}

	Vector2f Transform::TransformPoint(Vector2f position) const
	{
		// L'ordre des transformations s'appelle ici le SRT et est très courant

		// Scale
		position *= GetGlobalScale();

		// Rotation
		position = Vector2f::Rotate(position, GetGlobalRotation());

		// Translation
		if (m_parent)
			position += m_parent->TransformPoint(m_position);
		else
			position += m_position;

		return position;
	}

	Vector2f Transform::TransformInversePoint(Vector2f position) const
	{
		// Lorsqu'on effectue l'inverse d'une transformation, l'ordre de celles-ci est également inversé, on fait alors du TRS

		// Translation
		if (m_parent)
			position -= m_parent->TransformPoint(m_position);
		else
			position -= m_position;

		// Rotation
		position = Vector2f::Rotate(position, -GetGlobalRotation());

		// Scale
		position /= GetGlobalScale();

		return position;
	}

	Transform& Transform::operator=(const Transform& transform)
	{
		m_position = transform.m_position;
		m_rotation = transform.m_rotation;
		m_scale = transform.m_scale;
		SetParent(transform.m_parent);

		return *this;
	}

	Transform& Transform::operator=(Transform&& transform) noexcept
	{
		for (Transform* child : m_children)
			child->m_parent = nullptr;

		m_children = std::move(transform.m_children);
		m_position = transform.m_position;
		m_rotation = transform.m_rotation;
		m_scale = transform.m_scale;
		SetParent(transform.m_parent);

		for (Transform* child : m_children)
			child->m_parent = this;

		return *this;
	}

	void Transform::Unserialize(entt::handle entity, const nlohmann::json& doc)
	{
		auto& node = entity.emplace<Transform>();
		node.SetPosition(doc.value("Position", Vector2f(0, 0)));
		node.SetRotation(doc.value("Rotation", 0.f));
		node.SetScale(doc.value("Scale", Vector2f(1.f, 1.f)));
	}

	void Transform::AttachChild(Transform* child)
	{
		m_children.push_back(child);
	}

	void Transform::DetachChild(Transform* child)
	{
		auto it = std::find(m_children.begin(), m_children.end(), child);
		assert(it != m_children.end());

		m_children.erase(it);
	}
}