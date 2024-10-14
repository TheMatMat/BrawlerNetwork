#include <Sel/Vector4.hpp>
#include <Sel/Math.hpp>
#include <cmath>

namespace Sel
{
	template<typename T>
	Vector4<T>::Vector4(T V) :
	x(V),
	y(V),
	z(V),
	w(V)
	{
	}

	template<typename T>
	Vector4<T>::Vector4(T X, T Y, T Z, T W) :
	x(X),
	y(Y),
	z(Z),
	w(W)
	{
	}

	template<typename T>
	Vector4<T>::Vector4(const Vector3<T>& vec, T W) :
	x(vec.x),
	y(vec.y),
	z(vec.z),
	w(W)
	{
	}

	template<typename T>
	Vector4<T> Vector4<T>::operator+(const Vector4& vec) const
	{
		return Vector4{ x + vec.x, y + vec.y, z + vec.z, w + vec.w };
	}

	template<typename T>
	Vector4<T> Vector4<T>::operator-(const Vector4& vec) const
	{
		return Vector4{ x - vec.x, y - vec.y, z - vec.z, w - vec.w };
	}

	template<typename T>
	Vector4<T> Vector4<T>::operator*(const Vector4& vec) const
	{
		return Vector4{ x * vec.x, y * vec.y, z * vec.z, w * vec.w };
	}

	template<typename T>
	Vector4<T> Vector4<T>::operator*(T value) const
	{
		return Vector4{ x * value, y * value, z * value, w * value };
	}

	template<typename T>
	Vector4<T> Vector4<T>::operator/(const Vector4& vec) const
	{
		return Vector4{ x / vec.x, y / vec.y, z / vec.z, w / vec.w };
	}

	template<typename T>
	Vector4<T> Vector4<T>::operator/(T value) const
	{
		return Vector4{ x / value, y / value, z / value, w / value };
	}

	template<typename T>
	Vector4<T>& Vector4<T>::operator+=(const Vector4& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		w += vec.w;

		return *this;
	}

	template<typename T>
	Vector4<T>& Vector4<T>::operator-=(const Vector4& vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		w -= vec.w;

		return *this;
	}

	template<typename T>
	Vector4<T>& Vector4<T>::operator*=(const Vector4& vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		w *= vec.w;

		return *this;
	}

	template<typename T>
	Vector4<T>& Vector4<T>::operator*=(T value)
	{
		x *= value;
		y *= value;
		z *= value;
		w *= value;

		return *this;
	}

	template<typename T>
	Vector4<T>& Vector4<T>::operator/=(const Vector4& vec)
	{
		x /= vec.x;
		y /= vec.y;
		z /= vec.z;
		w /= vec.w;

		return *this;
	}

	template<typename T>
	Vector4<T>& Vector4<T>::operator/=(T value)
	{
		x /= value;
		y /= value;
		z /= value;
		w /= value;

		return *this;
	}

	template<typename T>
	Vector4<T> operator*(T value, const Vector4<T>& vec)
	{
		return Vector4{ vec.x * value, vec.y * value, vec.z * value, vec.w * value };
	}

	template<typename T>
	Vector4<T> operator/(T value, const Vector4<T>& vec)
	{
		return Vector4{ vec.x / value, vec.y / value, vec.z / value, vec.w / value };
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const Vector4<T>& vec)
	{
		return os << "Vector4(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	}
}
