#pragma once

#include <ostream>

namespace Sel
{
	// Pas de macro d'export pour les templates (leur code n'étant pas compilé dans la dll)
	template<typename T>
	struct Vector3
	{
		Vector3() = default; // valeur non-initialisée
		explicit Vector3(T V); //< explicit permet d'éviter les conversions implicites, sans ça on aurait pu écrire Vector3 x = 42, ou passer un entier à une fonction attendant un Vector3
		Vector3(T X, T Y, T Z);

		Vector3 operator+(const Vector3& vec) const;
		Vector3 operator-(const Vector3& vec) const;
		Vector3 operator*(const Vector3& vec) const;
		Vector3 operator*(T value) const;
		Vector3 operator/(const Vector3& vec) const;
		Vector3 operator/(T value) const;

		Vector3& operator+=(const Vector3& vec);
		Vector3& operator-=(const Vector3& vec);
		Vector3& operator*=(const Vector3& vec);
		Vector3& operator*=(T value);
		Vector3& operator/=(const Vector3& vec);
		Vector3& operator/=(T value);

		T x, y, z;
	};

	template<typename T> Vector3<T> operator*(T value, const Vector3<T>& vec);
	template<typename T> Vector3<T> operator/(T value, const Vector3<T>& vec);

	// Opérateur de flux, permet d'écrire un Vector3 directement dans std::cout (ou autre flux de sortie)
	template<typename T> std::ostream& operator<<(std::ostream& os, const Vector3<T>& vec);

	// On peut utiliser using pour faire un alias plus facile à écrire
	using Vector3f = Vector3<float>;
	using Vector3i = Vector3<int>;
}

// Il est relativement courant de séparer le code template dans un fichier annexe, pour clarifier les headers
// cela peut se faire en incluant un autre fichier (rappel, #include = copier-coller)
#include <Sel/Vector3.inl>
