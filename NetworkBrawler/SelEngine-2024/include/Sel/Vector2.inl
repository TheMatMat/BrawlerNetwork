// le #pragma once n'est pas nécessaire ici, un seul fichier va nous inclure directement et il est déjà protégé

#include <Sel/Vector2.hpp>
#include <Sel/Math.hpp>
#include <cmath>

namespace Sel
{
	template<typename T>
	Vector2<T>::Vector2(T V) :
	x(V),
	y(V)
	{
	}

	template<typename T>
	Vector2<T>::Vector2(T X, T Y) :
	x(X),
	y(Y)
	{
	}

	template<typename T>
	T Vector2<T>::Magnitude() const
	{
		return std::sqrt(SquaredMagnitude());
	}

	template<typename T>
	T Vector2<T>::SquaredMagnitude() const
	{
		return x * x + y * y;
	}

	template<typename T>
	Vector2<T> Vector2<T>::operator-() const
	{
		return Vector2(-x, -y);
	}

	template<typename T>
	Vector2<T> Vector2<T>::operator+(const Vector2& vec) const
	{
		return Vector2{ x + vec.x, y + vec.y };
	}

	template<typename T>
	Vector2<T> Vector2<T>::operator-(const Vector2& vec) const
	{
		return Vector2{ x - vec.x, y - vec.y };
	}

	template<typename T>
	Vector2<T> Vector2<T>::operator*(const Vector2& vec) const
	{
		return Vector2{ x * vec.x, y * vec.y };
	}

	template<typename T>
	Vector2<T> Vector2<T>::operator*(T value) const
	{
		return Vector2{ x * value, y * value };
	}

	template<typename T>
	Vector2<T> Vector2<T>::operator/(const Vector2& vec) const
	{
		return Vector2{ x / vec.x, y / vec.y };
	}

	template<typename T>
	Vector2<T> Vector2<T>::operator/(T value) const
	{
		return Vector2{ x / value, y / value };
	}

	template<typename T>
	Vector2<T>& Vector2<T>::operator+=(const Vector2& vec)
	{
		x += vec.x;
		y += vec.y;

		return *this;
	}

	template<typename T>
	Vector2<T>& Vector2<T>::operator-=(const Vector2& vec)
	{
		x -= vec.x;
		y -= vec.y;

		return *this;
	}

	template<typename T>
	Vector2<T>& Vector2<T>::operator*=(const Vector2& vec)
	{
		x *= vec.x;
		y *= vec.y;

		return *this;
	}

	template<typename T>
	Vector2<T>& Vector2<T>::operator*=(T value)
	{
		x *= value;
		y *= value;

		return *this;
	}

	template<typename T>
	Vector2<T>& Vector2<T>::operator/=(const Vector2& vec)
	{
		x /= vec.x;
		y /= vec.y;

		return *this;
	}

	template<typename T>
	Vector2<T>& Vector2<T>::operator/=(T value)
	{
		x /= value;
		y /= value;

		return *this;
	}

	template<typename T>
	float Vector2<T>::Distance(const Vector2& vec, const Vector2& vec2)
	{
		return (vec2 - vec).Magnitude();
	}

	template<typename T>
	Vector2<T> Vector2<T>::Normal(const Vector2& vec)
	{
		return vec / vec.Magnitude();
	}

	template<typename T>
	Vector2<T> Vector2<T>::Rotate(const Vector2& vec, float degrees)
	{
		float radRotation = Deg2Rad * degrees;
		float s = std::sin(radRotation);
		float c = std::cos(radRotation);

		Vector2 rotatedVec;
		rotatedVec.x = vec.x * c - vec.y * s;
		rotatedVec.y = vec.x * s + vec.y * c;

		return rotatedVec;
	}

	template<typename T>
	float Vector2<T>::SquaredDistance(const Vector2& vec, const Vector2& vec2)
	{
		return (vec2 - vec).SquaredMagnitude();
	}

	template<typename T>
	Vector2<T> operator*(T value, const Vector2<T>& vec)
	{
		return Vector2{ vec.x * value, vec.y * value };
	}

	template<typename T>
	Vector2<T> operator/(T value, const Vector2<T>& vec)
	{
		return Vector2{ vec.x / value, vec.y / value };
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const Vector2<T>& vec)
	{
		return os << "Vector2(" << vec.x << ", " << vec.y << ")";
	}
}
