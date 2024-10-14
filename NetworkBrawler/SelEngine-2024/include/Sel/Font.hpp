#pragma once

#include <Sel/Asset.hpp>
#include <Sel/Color.hpp>
#include <Sel/Surface.hpp>
#include <cstdint>
#include <string>

typedef struct _TTF_Font TTF_Font;

namespace Sel
{
	class SEL_ENGINE_API Font : public Asset
	{
		public:
			Font(const Font&) = delete;
			Font(Font&& font) noexcept;
			~Font();

			Surface RenderUTF8Text(int characterSize, const std::string& text, const Color& color = Color::White);

			Font& operator=(const Font&) = delete;
			Font& operator=(Font&& font) noexcept;

			static Font OpenFromFile(const std::string& filepath);

		private:
			explicit Font(TTF_Font* surface, std::string filepath);

			TTF_Font* m_font;
	};
}