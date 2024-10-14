#pragma once

#include <Sel/Export.hpp>
#include <Sel/ChipmunkBody.hpp>
#include <Sel/ChipmunkShape.hpp>
#include <memory>
#include <unordered_map>

namespace Sel
{
	class ChipmunkShape;
	struct CollisionShape;

	class SEL_ENGINE_API RigidBodyComponent
	{
		public:
			// Petite astuce, on peut se servir de structures vides comme de "tags", une façon de différencier les constructeurs
			struct Kinematic {};
			struct Static {};

			RigidBodyComponent(float mass, float moment = 1.f);
			RigidBodyComponent(Kinematic /*unnamed*/);
			RigidBodyComponent(Static);

			void AddShape(std::shared_ptr<CollisionShape> shape, const Vector2f& offset = Vector2f(0.f, 0.f), bool recomputeMoment = true);

			float GetAngularVelocity() const;
			ChipmunkBody& GetBody();
			Vector2f GetCenterOfGravity() const;
			Vector2f GetLinearVelocity() const;
			Vector2f GetPosition() const;
			float GetRotation() const;

			void RemoveShape(const std::shared_ptr<CollisionShape>& shape, bool recomputeMoment = true);

			void SetAngularVelocity(float angularVelocity);
			void SetCenterOfGravity(const Vector2f& centerOfGravity);
			void SetLinearVelocity(const Vector2f& linearVelocity);
			void SetMass(float mass, bool recomputeMoment = true);
			void SetVelocityFunction(ChipmunkBody::VelocityFunction velocityFunction);

			void TeleportTo(const Vector2f& position);
			void TeleportTo(const Vector2f& position, float rotation);

		private:
			void RecomputeMoment();
			
			struct ShapeData
			{
				ChipmunkShape physicsShape;
				Vector2f offset;
			};

			// L'ordre des membres est important : ils sont construits dans l'ordre de déclaration
			// et détruits dans l'ordre inverse
			// ainsi ici, les shapes seront détruits avant que le body ne le soit (l'inverse poserait problème !)
			ChipmunkBody m_body;
			std::unordered_map<std::shared_ptr<CollisionShape>, ShapeData> m_shapes;
	};	
}