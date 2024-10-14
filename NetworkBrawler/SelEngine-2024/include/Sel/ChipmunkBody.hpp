#pragma once

#include <Sel/Export.hpp>
#include <Sel/Vector2.hpp>
#include <functional>

struct cpBody;

namespace Sel
{
	class ChipmunkSpace;

	class SEL_ENGINE_API ChipmunkBody
	{
		public:
			using VelocityFunction = std::function<void(ChipmunkBody& body, const Vector2f& gravity, float damping, float deltatime)>;

			ChipmunkBody(const ChipmunkBody&) = delete;
			ChipmunkBody(ChipmunkBody&& body) noexcept;
			~ChipmunkBody();

			void ApplyImpulse(const Vector2f& impulse);
			void ApplyImpulseAtWorldPoint(const Vector2f& impulse, const Vector2f& worldPoint);

			float GetAngularVelocity() const;
			Vector2f GetCenterOfGravity() const;
			Vector2f GetDirection() const;
			cpBody* GetHandle() const;
			Vector2f GetLinearVelocity() const;
			float GetMass() const;
			float GetMoment() const;
			Vector2f GetPosition() const;
			float GetRotation() const;

			bool IsDynamic() const;
			bool IsKinematic() const;
			bool IsStatic() const;

			void ReindexShapes();

			void SetAngularVelocity(float angularVel);
			void SetCenterOfGravity(const Vector2f& centerOfMass);
			void SetLinearVelocity(const Vector2f& velocity);
			void SetMass(float mass);
			void SetMoment(float moment);
			void SetPosition(const Vector2f& pos);
			void SetRotation(float rotation);
			void SetVelocityFunction(VelocityFunction velocityFunction);

			void UpdateVelocity(const Vector2f& gravity, float damping, float deltatime);

			ChipmunkBody& operator=(const ChipmunkBody&) = delete;
			ChipmunkBody& operator=(ChipmunkBody&& body) noexcept;

			static ChipmunkBody Build(ChipmunkSpace& space, float mass, float moment);
			static ChipmunkBody BuildKinematic(ChipmunkSpace& space);
			static ChipmunkBody BuildStatic(ChipmunkSpace& space);

		private:
			ChipmunkBody(ChipmunkSpace& space, cpBody* body);

			VelocityFunction m_velocityFunction;
			cpBody* m_handle;
	};	
}