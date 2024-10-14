#include <Sel/Matrix4.hpp>
#include <Sel/Math.hpp>
#include <cmath>

namespace Sel
{
	template<typename T>
	Matrix4<T>::Matrix4(std::array<T, 4 * 4> values) :
	m_values(values)
	{
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::Inverse() const
	{
		// https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix

		T A2323 = Value(2, 2) * Value(3, 3) - Value(2, 3) * Value(3, 2);
		T A1323 = Value(2, 1) * Value(3, 3) - Value(2, 3) * Value(3, 1);
		T A1223 = Value(2, 1) * Value(3, 2) - Value(2, 2) * Value(3, 1);
		T A0323 = Value(2, 0) * Value(3, 3) - Value(2, 3) * Value(3, 0);
		T A0223 = Value(2, 0) * Value(3, 2) - Value(2, 2) * Value(3, 0);
		T A0123 = Value(2, 0) * Value(3, 1) - Value(2, 1) * Value(3, 0);
		T A2313 = Value(1, 2) * Value(3, 3) - Value(1, 3) * Value(3, 2);
		T A1313 = Value(1, 1) * Value(3, 3) - Value(1, 3) * Value(3, 1);
		T A1213 = Value(1, 1) * Value(3, 2) - Value(1, 2) * Value(3, 1);
		T A2312 = Value(1, 2) * Value(2, 3) - Value(1, 3) * Value(2, 2);
		T A1312 = Value(1, 1) * Value(2, 3) - Value(1, 3) * Value(2, 1);
		T A1212 = Value(1, 1) * Value(2, 2) - Value(1, 2) * Value(2, 1);
		T A0313 = Value(1, 0) * Value(3, 3) - Value(1, 3) * Value(3, 0);
		T A0213 = Value(1, 0) * Value(3, 2) - Value(1, 2) * Value(3, 0);
		T A0312 = Value(1, 0) * Value(2, 3) - Value(1, 3) * Value(2, 0);
		T A0212 = Value(1, 0) * Value(2, 2) - Value(1, 2) * Value(2, 0);
		T A0113 = Value(1, 0) * Value(3, 1) - Value(1, 1) * Value(3, 0);
		T A0112 = Value(1, 0) * Value(2, 1) - Value(1, 1) * Value(2, 0);

		T det = Value(0, 0) * (Value(1, 1) * A2323 - Value(1, 2) * A1323 + Value(1, 3) * A1223)
		      - Value(0, 1) * (Value(1, 0) * A2323 - Value(1, 2) * A0323 + Value(1, 3) * A0223)
		      + Value(0, 2) * (Value(1, 0) * A1323 - Value(1, 1) * A0323 + Value(1, 3) * A0123)
		      - Value(0, 3) * (Value(1, 0) * A1223 - Value(1, 1) * A0223 + Value(1, 2) * A0123);
		det = 1.0f / det;

		Matrix4 result;
		result(0, 0) = det *  (Value(1, 1) * A2323 - Value(1, 2) * A1323 + Value(1, 3) * A1223);
		result(0, 1) = det * -(Value(0, 1) * A2323 - Value(0, 2) * A1323 + Value(0, 3) * A1223);
		result(0, 2) = det *  (Value(0, 1) * A2313 - Value(0, 2) * A1313 + Value(0, 3) * A1213);
		result(0, 3) = det * -(Value(0, 1) * A2312 - Value(0, 2) * A1312 + Value(0, 3) * A1212);
		result(1, 0) = det * -(Value(1, 0) * A2323 - Value(1, 2) * A0323 + Value(1, 3) * A0223);
		result(1, 1) = det *  (Value(0, 0) * A2323 - Value(0, 2) * A0323 + Value(0, 3) * A0223);
		result(1, 2) = det * -(Value(0, 0) * A2313 - Value(0, 2) * A0313 + Value(0, 3) * A0213);
		result(1, 3) = det *  (Value(0, 0) * A2312 - Value(0, 2) * A0312 + Value(0, 3) * A0212);
		result(2, 0) = det *  (Value(1, 0) * A1323 - Value(1, 1) * A0323 + Value(1, 3) * A0123);
		result(2, 1) = det * -(Value(0, 0) * A1323 - Value(0, 1) * A0323 + Value(0, 3) * A0123);
		result(2, 2) = det *  (Value(0, 0) * A1313 - Value(0, 1) * A0313 + Value(0, 3) * A0113);
		result(2, 3) = det * -(Value(0, 0) * A1312 - Value(0, 1) * A0312 + Value(0, 3) * A0112);
		result(3, 0) = det * -(Value(1, 0) * A1223 - Value(1, 1) * A0223 + Value(1, 2) * A0123);
		result(3, 1) = det *  (Value(0, 0) * A1223 - Value(0, 1) * A0223 + Value(0, 2) * A0123);
		result(3, 2) = det * -(Value(0, 0) * A1213 - Value(0, 1) * A0213 + Value(0, 2) * A0113);
		result(3, 3) = det *  (Value(0, 0) * A1212 - Value(0, 1) * A0212 + Value(0, 2) * A0112);

		return result;
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::Transpose() const
	{
		Matrix4 mat;
		for (std::size_t i = 0; i < 4; ++i)
		{
			for (std::size_t j = 0; j < 4; ++j)
				mat(j, i) = Value(i, j);
		}

		return mat;
	}

	template<typename T>
	T& Matrix4<T>::Value(std::size_t i, std::size_t j)
	{
		return m_values[i * 4 + j];
	}

	template<typename T>
	const T& Matrix4<T>::Value(std::size_t i, std::size_t j) const
	{
		return m_values[i * 4 + j];
	}

	template<typename T>
	T& Matrix4<T>::operator()(std::size_t i, std::size_t j)
	{
		return Value(i, j);
	}

	template<typename T>
	const T& Matrix4<T>::operator()(std::size_t i, std::size_t j) const
	{
		return Value(i, j);
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::operator*(const Matrix4& rhs) const
	{
		Matrix4 result;
		for (std::size_t i = 0; i < 4; ++i)
		{
			for (std::size_t j = 0; j < 4; ++j)
			{
				T sum = 0.0f;
				for (std::size_t k = 0; k < 4; ++k)
					sum += Value(i, k) * rhs(k, j);

				result(i, j) = sum;
			}
		}

		return result;
	}

	template<typename T>
	Vector2<T> Matrix4<T>::operator*(const Vector2<T>& vec) const
	{
		Vector2<T> result;
		result.x = Value(0, 0) * vec.x + Value(0, 1) * vec.y + Value(0, 3);
		result.y = Value(1, 0) * vec.x + Value(1, 1) * vec.y + Value(1, 3);

		return result;
	}

	template<typename T>
	Vector3<T> Matrix4<T>::operator*(const Vector3<T>& vec) const
	{
		Vector3<T> result;
		result.x = Value(0, 0) * vec.x + Value(0, 1) * vec.y + Value(0, 2) * vec.z + Value(0, 3);
		result.y = Value(1, 0) * vec.x + Value(1, 1) * vec.y + Value(1, 2) * vec.z + Value(1, 3);
		result.z = Value(2, 0) * vec.x + Value(2, 1) * vec.y + Value(2, 2) * vec.z + Value(2, 3);

		return result;
	}

	template<typename T>
	Vector4<T> Matrix4<T>::operator*(const Vector4<T>& vec) const
	{
		Vector4<T> result;
		result.x = Value(0, 0) * vec.x + Value(0, 1) * vec.y + Value(0, 2) * vec.z + Value(0, 3) * vec.w;
		result.y = Value(1, 0) * vec.x + Value(1, 1) * vec.y + Value(1, 2) * vec.z + Value(1, 3) * vec.w;
		result.z = Value(2, 0) * vec.x + Value(2, 1) * vec.y + Value(2, 2) * vec.z + Value(2, 3) * vec.w;
		result.w = Value(3, 0) * vec.x + Value(3, 1) * vec.y + Value(3, 2) * vec.z + Value(3, 3) * vec.w;

		return result;
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::Identity()
	{
		return Matrix4({
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		});
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::Perspective(float fov, float ratio, float zNear, float zFar)
	{
		float halfFov = fov * 0.5f;
		float yScale = std::tan(halfFov * Sel::Deg2Rad);

		return Matrix4({
			1.f / (ratio * yScale), 0.f,           0.f,                   0.f,
			0.f,                    -1.f / yScale, 0.f,                   0.f,
			0.f,                    0.f,           zFar / (zNear - zFar), -(zNear * zFar) / (zFar - zNear),
			0.f,                    0.f,           -1.f,                  0.f
		});
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::RotateAroundX(float degreeAngle)
	{
		float sinAngle = std::sin(degreeAngle * Deg2Rad);
		float cosAngle = std::cos(degreeAngle * Deg2Rad);

		return Matrix4({
			1.0f,      0.0f,     0.0f,  0.0f,
			0.0f,  cosAngle, -sinAngle, 0.0f,
			0.0f,  sinAngle, cosAngle,  0.0f,
			0.0f,      0.0f,     0.0f,  1.0f
		});
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::RotateAroundY(float degreeAngle)
	{
		float sinAngle = std::sin(degreeAngle * Deg2Rad);
		float cosAngle = std::cos(degreeAngle * Deg2Rad);

		return Matrix4({
			cosAngle,  0.0f, -sinAngle, 0.0f,
			0.0f,      1.0f,      0.0f, 0.0f,
			sinAngle,  0.0f,  cosAngle, 0.0f,
			0.0f,      0.0f,     0.0f,  1.0f
		});
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::RotateAroundZ(float degreeAngle)
	{
		float sinAngle = std::sin(degreeAngle * Deg2Rad);
		float cosAngle = std::cos(degreeAngle * Deg2Rad);

		return Matrix4({
			cosAngle,  sinAngle, 0.0f, 0.0f,
			-sinAngle, cosAngle, 0.0f, 0.0f,
			0.0f,          0.0f, 1.0f, 0.0f,
			0.0f,          0.0f, 0.0f, 1.0f
		});
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::Scale(const Vector3<T>& scale)
	{
		return Matrix4({
			scale.x, 0.0f,    0.0f,	   0.0f,
			0.0f,    scale.y, 0.0f,	   0.0f,
			0.f,     0.0f,    scale.z, 0.0f,
			0.0f,    0.0f,    0.0f,    1.0f
		});
	}

	template<typename T>
	Matrix4<T> Matrix4<T>::Translate(const Vector3<T>& translation)
	{
		return Matrix4({
			1.0f, 0.0f, 0.0f, translation.x,
			0.0f, 1.0f, 0.0f, translation.y,
			0.0f, 0.0f, 1.0f, translation.z,
			0.0f, 0.0f, 0.0f, 1.0f
		});
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const Matrix4<T>& mat)
	{
		return os << "Matrix4(" << mat.Value(0, 0) << ", " << mat.Value(0, 1) << ", " << mat.Value(0, 2) << ", " << mat.Value(0, 3) << "\n"
		        << "          " << mat.Value(1, 0) << ", " << mat.Value(1, 1) << ", " << mat.Value(1, 2) << ", " << mat.Value(1, 3) << "\n"
		        << "          " << mat.Value(2, 0) << ", " << mat.Value(2, 1) << ", " << mat.Value(2, 2) << ", " << mat.Value(2, 3) << "\n"
		        << "          " << mat.Value(3, 0) << ", " << mat.Value(3, 1) << ", " << mat.Value(3, 2) << ", " << mat.Value(3, 3) << ")";
	}
}
