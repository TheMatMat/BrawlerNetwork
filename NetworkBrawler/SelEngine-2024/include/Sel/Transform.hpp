#pragma once

#include <Sel/Export.hpp>
#include <Sel/Matrix3.hpp>
#include <Sel/Vector2.hpp>
#include <entt/fwd.hpp>
#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace Sel
{
	class WorldEditor;

	class SEL_ENGINE_API Transform
	{
		public:
			Transform();
			Transform(const Transform& transform);
			Transform(Transform&&) noexcept;
			~Transform();

			Vector2f GetGlobalPosition() const;
			float GetGlobalRotation() const;
			Vector2f GetGlobalScale() const;

			const std::vector<Transform*>& GetChildren() const;
			Transform* GetParent() const;
			const Vector2f& GetPosition() const;
			float GetRotation() const;
			const Vector2f& GetScale() const;
			Matrix3f GetTransformMatrix() const;

			void PopulateInspector(WorldEditor& worldEditor);

			void Rotate(float rotation);
			void Scale(float scale);
			void Scale(const Vector2f& scale);

			nlohmann::json Serialize(const entt::handle entity) const;

			void SetParent(Transform* parent);
			void SetPosition(const Vector2f& position);
			void SetRotation(float rotation);
			void SetScale(const Vector2f& scale);

			void Translate(const Vector2f& translation);

			Vector2f TransformPoint(Vector2f position) const;
			Vector2f TransformInversePoint(Vector2f position) const;

			Transform& operator=(const Transform&);
			Transform& operator=(Transform&&) noexcept;

			static void Unserialize(entt::handle entity, const nlohmann::json& doc);

		private:
			void AttachChild(Transform* child);
			void DetachChild(Transform* child);

			std::vector<Transform*> m_children;
			Transform* m_parent;
			Vector2f m_position;
			float m_rotation;
			Vector2f m_scale;
	};
}
