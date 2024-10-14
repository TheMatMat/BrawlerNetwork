#pragma once

#include <Sel/Export.hpp>
#include <Sel/Matrix3.hpp>
#include <Sel/Vector2.hpp>

struct cpSpace;

namespace Sel
{
	class ChipmunkShape;
	class Renderer;

	class SEL_ENGINE_API ChipmunkSpace
	{
		public:
			ChipmunkSpace();
			ChipmunkSpace(const ChipmunkSpace&) = delete;
			ChipmunkSpace(ChipmunkSpace&& space) noexcept;
			~ChipmunkSpace();

			void DebugDraw(Renderer& renderer, const Matrix3f& cameraInverseTransform);

			cpSpace* GetHandle() const;

			ChipmunkShape* PointQueryNearest(const Vector2f& point, float maxDist);

			void SetDamping(float damping);
			void SetGravity(const Vector2f& gravity);

			void Step(float deltaTime);

			ChipmunkSpace& operator=(const ChipmunkSpace&) = delete;
			ChipmunkSpace& operator=(ChipmunkSpace&& space) noexcept;

		private:
			cpSpace* m_handle;
	};	
}