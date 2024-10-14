#pragma once

#include <Sel/Vector2.hpp>
#include <Sel/Vector3.hpp>
#include <Sel/Vector4.hpp>
#include <array>
#include <ostream>

namespace Sel
{
	// Pas de macro d'export pour les templates (leur code n'étant pas compilé dans la dll)
	template<typename T>
	class Matrix4
	{
		public:
			Matrix4() = default;
			Matrix4(std::array<T, 4 * 4> values);

			Matrix4 Inverse() const;

			Matrix4 Transpose() const;

			T& Value(std::size_t i, std::size_t j);
			const T& Value(std::size_t i, std::size_t j) const;

			T& operator()(std::size_t i, std::size_t j);
			const T& operator()(std::size_t i, std::size_t j) const;

			Matrix4 operator*(const Matrix4& rhs) const;
			Vector2<T> operator*(const Vector2<T>& vec) const;
			Vector3<T> operator*(const Vector3<T>& vec) const;
			Vector4<T> operator*(const Vector4<T>& vec) const;

			static Matrix4 Identity();
			static Matrix4 Perspective(float fov, float ratio, float zNear, float zFar);
			static Matrix4 RotateAroundX(float degreeAngle);
			static Matrix4 RotateAroundY(float degreeAngle);
			static Matrix4 RotateAroundZ(float degreeAngle);
			static Matrix4 Scale(const Vector3<T>& scale);
			static Matrix4 Translate(const Vector3<T>& translation);

		private:
			std::array<T, 4 * 4> m_values;
	};

	// Opérateur de flux, permet d'écrire un Matrix4 directement dans std::cout (ou autre flux de sortie)
	template<typename T> std::ostream& operator<<(std::ostream& os, const Matrix4<T>& mat);

	// On peut utiliser using pour faire un alias plus facile à écrire
	using Matrix4f = Matrix4<float>;
}

// Il est relativement courant de séparer le code template dans un fichier annexe, pour clarifier les headers
// cela peut se faire en incluant un autre fichier (rappel, #include = copier-coller)
#include <Sel/Matrix4.inl>
