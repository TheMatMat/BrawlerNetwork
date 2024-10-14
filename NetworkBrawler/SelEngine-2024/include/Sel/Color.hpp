#pragma once

#include <Sel/Export.hpp>
#include <cstdint>
#include <ostream>

namespace Sel
{
	struct SEL_ENGINE_API Color
	{
		Color();
		Color(float red, float green, float blue, float alpha = 1.f);

		void ToRGBA8(std::uint8_t& red, std::uint8_t& green, std::uint8_t& blue, std::uint8_t& alpha) const;

		float r, g, b, a;

		static Color FromRGBA8(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255);

		static const Color White;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Yellow;
		static const Color Cyan;
	};

	// Opérateur de flux, permet d'écrire un Color directement dans std::cout (ou autre flux de sortie)
	SEL_ENGINE_API std::ostream& operator<<(std::ostream& os, const Color& color);
}