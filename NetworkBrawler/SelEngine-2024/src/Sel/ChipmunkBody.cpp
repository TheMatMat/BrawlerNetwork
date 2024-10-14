#include <Sel/ChipmunkBody.hpp>
#include <Sel/ChipmunkSpace.hpp>
#include <Sel/Math.hpp>
#include <chipmunk/chipmunk.h>
#include <algorithm>

namespace Sel
{
	ChipmunkBody::ChipmunkBody(ChipmunkSpace& space, cpBody* body) :
	m_handle(body)
	{
		cpSpaceAddBody(space.GetHandle(), m_handle);
		cpBodySetUserData(m_handle, this);
	}

	ChipmunkBody::ChipmunkBody(ChipmunkBody&& body) noexcept
	{
		m_handle = body.m_handle;
		body.m_handle = nullptr;

		if (m_handle)
			cpBodySetUserData(m_handle, this);
	}

	ChipmunkBody::~ChipmunkBody()
	{
		if (m_handle)
		{
			// On désenregistre le body du space automatiquement avant de le détruire
			if (cpSpace* space = cpBodyGetSpace(m_handle))
				cpSpaceRemoveBody(space, m_handle);

			cpBodyFree(m_handle);
		}
	}

	void ChipmunkBody::ApplyImpulse(const Vector2f& impulse)
	{
		cpBodyApplyImpulseAtWorldPoint(m_handle, cpv(impulse.x, impulse.y), cpBodyGetPosition(m_handle));
	}

	void ChipmunkBody::ApplyImpulseAtWorldPoint(const Vector2f& impulse, const Vector2f& worldPoint)
	{
		cpBodyApplyImpulseAtWorldPoint(m_handle, cpv(impulse.x, impulse.y), cpv(worldPoint.x, worldPoint.y));
	}

	float ChipmunkBody::GetAngularVelocity() const
	{
		return static_cast<float>(cpBodyGetAngularVelocity(m_handle) * Rad2Deg);
	}

	Vector2f ChipmunkBody::GetCenterOfGravity() const
	{
		cpVect cog = cpBodyGetCenterOfGravity(m_handle);
		return Vector2f(static_cast<float>(cog.x), static_cast<float>(cog.y));
	}

	Vector2f ChipmunkBody::GetDirection() const
	{
		cpVect rotation = cpBodyGetRotation(m_handle);
		return Vector2f(static_cast<float>(rotation.x), static_cast<float>(rotation.y));
	}

	cpBody* ChipmunkBody::GetHandle() const
	{
		return m_handle;
	}

	Vector2f ChipmunkBody::GetLinearVelocity() const
	{
		cpVect velocity = cpBodyGetVelocity(m_handle);
		return Vector2f(static_cast<float>(velocity.x), static_cast<float>(velocity.y));
	}

	float ChipmunkBody::GetMass() const
	{
		return static_cast<float>(cpBodyGetMass(m_handle));
	}

	float ChipmunkBody::GetMoment() const
	{
		return static_cast<float>(cpBodyGetMoment(m_handle));
	}

	Vector2f ChipmunkBody::GetPosition() const
	{
		cpVect position = cpBodyGetPosition(m_handle);
		return Vector2f(static_cast<float>(position.x), static_cast<float>(position.y));
	}

	float ChipmunkBody::GetRotation() const
	{
		// On inverse car notre repère n'est pas mathématique
		return -static_cast<float>(cpBodyGetAngle(m_handle) * Rad2Deg);
	}

	bool ChipmunkBody::IsDynamic() const
	{
		return cpBodyGetType(m_handle) == cpBodyType::CP_BODY_TYPE_DYNAMIC;
	}

	bool ChipmunkBody::IsKinematic() const
	{
		return cpBodyGetType(m_handle) == cpBodyType::CP_BODY_TYPE_KINEMATIC;
	}

	bool ChipmunkBody::IsStatic() const
	{
		return cpBodyGetType(m_handle) == cpBodyType::CP_BODY_TYPE_STATIC;
	}

	void ChipmunkBody::ReindexShapes()
	{
		if (cpSpace* space = cpBodyGetSpace(m_handle))
			cpSpaceReindexShapesForBody(space, m_handle);
	}

	void ChipmunkBody::SetAngularVelocity(float angularVel)
	{
		cpBodySetAngularVelocity(m_handle, angularVel * Deg2Rad);
	}

	void ChipmunkBody::SetCenterOfGravity(const Vector2f& centerOfMass)
	{
		cpBodySetCenterOfGravity(m_handle, cpv(centerOfMass.x, centerOfMass.y));
	}

	void ChipmunkBody::SetLinearVelocity(const Vector2f& velocity)
	{
		cpBodySetVelocity(m_handle, cpv(velocity.x, velocity.y));
	}

	void ChipmunkBody::SetMass(float mass)
	{
		cpBodySetMass(m_handle, mass);
	}

	void ChipmunkBody::SetMoment(float moment)
	{
		cpBodySetMoment(m_handle, moment);
	}

	void ChipmunkBody::SetPosition(const Vector2f& pos)
	{
		cpBodySetPosition(m_handle, cpv(pos.x, pos.y));
	}

	void ChipmunkBody::SetRotation(float rotation)
	{
		// On inverse car notre repère n'est pas mathématique
		cpBodySetAngle(m_handle, -rotation * Deg2Rad);
	}

	void ChipmunkBody::SetVelocityFunction(VelocityFunction velocityFunction)
	{
		m_velocityFunction = std::move(velocityFunction);
		if (m_velocityFunction)
		{
			cpBodySetVelocityUpdateFunc(m_handle, [](cpBody* cpBody, cpVect gravity, cpFloat damping, cpFloat dt)
			{
				ChipmunkBody* body = static_cast<ChipmunkBody*>(cpBodyGetUserData(cpBody));
				body->m_velocityFunction(*body, Vector2f(float(gravity.x), float(gravity.y)), float(damping), float(dt));
			});
		}
		else
			cpBodySetVelocityUpdateFunc(m_handle, cpBodyUpdateVelocity);
	}

	void ChipmunkBody::UpdateVelocity(const Vector2f& gravity, float damping, float deltatime)
	{
		cpBodyUpdateVelocity(m_handle, cpv(gravity.x, gravity.y), damping, deltatime);
	}

	ChipmunkBody& ChipmunkBody::operator=(ChipmunkBody&& body) noexcept
	{
		std::swap(m_handle, body.m_handle);

		if (m_handle)
			cpBodySetUserData(m_handle, this);

		if (body.m_handle)
			cpBodySetUserData(body.m_handle, &body);

		return *this;
	}

	ChipmunkBody ChipmunkBody::Build(ChipmunkSpace& space, float mass, float moment)
	{
		return ChipmunkBody(space, cpBodyNew(mass, moment));
	}

	ChipmunkBody ChipmunkBody::BuildKinematic(ChipmunkSpace& space)
	{
		return ChipmunkBody(space, cpBodyNewKinematic());
	}

	ChipmunkBody ChipmunkBody::BuildStatic(ChipmunkSpace& space)
	{
		return ChipmunkBody(space, cpBodyNewStatic());
	}
}
