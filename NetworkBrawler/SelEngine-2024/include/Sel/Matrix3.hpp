#pragma once

#include <Sel/Vector2.hpp>
#include <array>
#include <ostream>

namespace Sel
{
	// Pas de macro d'export pour les templates (leur code n'étant pas compilé dans la dll)
	template<typename T>
	class Matrix3
	{
		public:
			Matrix3() = default;
			Matrix3(std::array<T, 3 * 3> values);

			T Determinant() const;

			Matrix3 GetInverse() const;
			Matrix3 GetTransposed() const;

			T& Value(std::size_t i, std::size_t j);
			const T& Value(std::size_t i, std::size_t j) const;

			T& operator()(std::size_t i, std::size_t j);
			const T& operator()(std::size_t i, std::size_t j) const;

			Matrix3 operator*(const Matrix3& rhs) const;
			Vector2<T> operator*(const Vector2<T>& vec) const;

			static Matrix3 Identity();
			static Matrix3 Rotate(float degreeAngle);
			static Matrix3 Scale(const Vector2<T>& scale);
			static Matrix3 Translate(const Vector2<T>& translation);

		private:
			std::array<T, 3 * 3> m_values;
	};

	// Opérateur de flux, permet d'écrire un Matrix3 directement dans std::cout (ou autre flux de sortie)
	template<typename T> std::ostream& operator<<(std::ostream& os, const Matrix3<T>& mat);

	// On peut utiliser using pour faire un alias plus facile à écrire
	using Matrix3f = Matrix3<float>;
}

// Il est relativement courant de séparer le code template dans un fichier annexe, pour clarifier les headers
// cela peut se faire en incluant un autre fichier (rappel, #include = copier-coller)
#include <Sel/Matrix3.inl>
