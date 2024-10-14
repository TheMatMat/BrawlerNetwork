#pragma once

#include <ostream>

namespace Sel
{
	template<typename T> struct Vector3;

	// Pas de macro d'export pour les templates (leur code n'étant pas compilé dans la dll)
	template<typename T>
	struct Vector4
	{
		Vector4() = default; // valeur non-initialisée
		explicit Vector4(T V); //< explicit permet d'éviter les conversions implicites, sans ça on aurait pu écrire Vector4 x = 42, ou passer un entier à une fonction attendant un Vector4
		Vector4(T X, T Y, T Z, T W);
		Vector4(const Vector3<T>& vec, T W);

		Vector4 operator+(const Vector4& vec) const;
		Vector4 operator-(const Vector4& vec) const;
		Vector4 operator*(const Vector4& vec) const;
		Vector4 operator*(T value) const;
		Vector4 operator/(const Vector4& vec) const;
		Vector4 operator/(T value) const;

		Vector4& operator+=(const Vector4& vec);
		Vector4& operator-=(const Vector4& vec);
		Vector4& operator*=(const Vector4& vec);
		Vector4& operator*=(T value);
		Vector4& operator/=(const Vector4& vec);
		Vector4& operator/=(T value);

		T x, y, z, w;
	};

	template<typename T> Vector4<T> operator*(T value, const Vector4<T>& vec);
	template<typename T> Vector4<T> operator/(T value, const Vector4<T>& vec);

	// Opérateur de flux, permet d'écrire un Vector4 directement dans std::cout (ou autre flux de sortie)
	template<typename T> std::ostream& operator<<(std::ostream& os, const Vector4<T>& vec);

	// On peut utiliser using pour faire un alias plus facile à écrire
	using Vector4f = Vector4<float>;
	using Vector4i = Vector4<int>;
}

// Il est relativement courant de séparer le code template dans un fichier annexe, pour clarifier les headers
// cela peut se faire en incluant un autre fichier (rappel, #include = copier-coller)
#include <Sel/Vector4.inl>
