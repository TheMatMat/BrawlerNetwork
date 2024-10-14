#include <Sel/CollisionShape.hpp>
#include <Sel/ChipmunkShape.hpp>
#include <Sel/Model.hpp>

namespace Sel
{
	CollisionShape::~CollisionShape() = default;

	//

	BoxShape::BoxShape(float width_, float height_, float radius_) :
	x(-width_ / 2.f),
	y(-height_ / 2.f),
	width(width_),
	height(height_),
	radius(radius_)
	{
	}

	BoxShape::BoxShape(float x_, float y_, float width_, float height_, float radius_) :
	x(x_),
	y(y_),
	width(width_),
	height(height_),
	radius(radius_)
	{
	}

	ChipmunkShape BoxShape::Build(ChipmunkBody& body, const Vector2f& offset) const
	{
		Vector2f topLeftCorner(x + offset.x, y + offset.y);
		Vector2f bottomRightCorner(x + width + offset.x, y + height + offset.y);

		ChipmunkShape shape = ChipmunkShape::BuildBox(body, topLeftCorner, bottomRightCorner, radius);
		shape.SetElasticty(elasticity);
		shape.SetFriction(friction);

		return shape;
	}

	float BoxShape::ComputeMoment(float mass, const Vector2f& offset) const
	{
		Vector2f topLeftCorner(x + offset.x, y + offset.y);
		Vector2f bottomRightCorner(x + width + offset.x, y + height + offset.y);

		return ChipmunkShape::ComputeBoxMoment(mass, topLeftCorner, bottomRightCorner);
	}

	//

	CircleShape::CircleShape(float radius_) :
	radius(radius_)
	{
	}

	ChipmunkShape CircleShape::Build(ChipmunkBody& body, const Vector2f& offset) const
	{
		ChipmunkShape shape = ChipmunkShape::BuildCircle(body, radius, offset);
		shape.SetElasticty(elasticity);
		shape.SetFriction(friction);

		return shape;
	}

	float CircleShape::ComputeMoment(float mass, const Vector2f& offset) const
	{
		return ChipmunkShape::ComputeCircleMoment(mass, radius, offset);
	}

	//

	ConvexShape::ConvexShape(std::vector<Vector2f> pos, float radius_) :
	positions(std::move(pos)),
	radius(radius_)
	{
	}

	ConvexShape::ConvexShape(const Model& model, Matrix3f transformation, float radius_) :
	radius(radius_)
	{
		const std::vector<ModelVertex>& vertices = model.GetVertices();

		positions.resize(vertices.size());
		for (std::size_t i = 0; i < vertices.size(); ++i)
			positions[i] = transformation * vertices[i].pos;
	}

	ChipmunkShape ConvexShape::Build(ChipmunkBody& body, const Vector2f& offset) const
	{
		ChipmunkShape shape = ChipmunkShape::BuildConvexHull(body, positions, Matrix3f::Translate(offset));
		shape.SetElasticty(elasticity);
		shape.SetFriction(friction);

		return shape;
	}

	float ConvexShape::ComputeMoment(float mass, const Vector2f& offset) const
	{
		return ChipmunkShape::ComputePolyMoment(mass, positions, Matrix3f::Translate(offset));
	}

	//

	SegmentShape::SegmentShape(const Vector2f& from_, const Vector2f& to_, float radius_) :
	from(from_),
	to(to_),
	radius(radius_)
	{
	}

	ChipmunkShape SegmentShape::Build(ChipmunkBody& body, const Vector2f& offset) const
	{
		ChipmunkShape shape = ChipmunkShape::BuildSegment(body, from + offset, to + offset, radius);
		shape.SetElasticty(elasticity);
		shape.SetFriction(friction);

		return shape;
	}

	float SegmentShape::ComputeMoment(float mass, const Vector2f& offset) const
	{
		return ChipmunkShape::ComputeSegmentMoment(mass, from + offset, to + offset, radius);
	}
}