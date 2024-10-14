#pragma once

#include <ostream>

namespace Sel
{
	// Pas de macro d'export pour les templates (leur code n'étant pas compilé dans la dll)
	template<typename T>
	struct Vector2
	{
		Vector2() = default; // valeur non-initialisée
		explicit Vector2(T V); //< explicit permet d'éviter les conversions implicites, sans ça on aurait pu écrire Vector2 x = 42, ou passer un entier à une fonction attendant un Vector2
		Vector2(T X, T Y);

		T Magnitude() const;
		T SquaredMagnitude() const;

		Vector2 operator-() const;

		Vector2 operator+(const Vector2& vec) const;
		Vector2 operator-(const Vector2& vec) const;
		Vector2 operator*(const Vector2& vec) const;
		Vector2 operator*(T value) const;
		Vector2 operator/(const Vector2& vec) const;
		Vector2 operator/(T value) const;

		Vector2& operator+=(const Vector2& vec);
		Vector2& operator-=(const Vector2& vec);
		Vector2& operator*=(const Vector2& vec);
		Vector2& operator*=(T value);
		Vector2& operator/=(const Vector2& vec);
		Vector2& operator/=(T value);

		static float Distance(const Vector2& vec, const Vector2& vec2);
		static Vector2 Normal(const Vector2& vec);
		static Vector2 Rotate(const Vector2& vec, float degrees);
		static float SquaredDistance(const Vector2& vec, const Vector2& vec2);

		T x, y;
	};

	template<typename T> Vector2<T> operator*(T value, const Vector2<T>& vec);
	template<typename T> Vector2<T> operator/(T value, const Vector2<T>& vec);

	// Opérateur de flux, permet d'écrire un Vector2 directement dans std::cout (ou autre flux de sortie)
	template<typename T> std::ostream& operator<<(std::ostream& os, const Vector2<T>& vec);

	// On peut utiliser using pour faire un alias plus facile à écrire
	using Vector2f = Vector2<float>;
	using Vector2i = Vector2<int>;
}

// Il est relativement courant de séparer le code template dans un fichier annexe, pour clarifier les headers
// cela peut se faire en incluant un autre fichier (rappel, #include = copier-coller)
#include <Sel/Vector2.inl>
