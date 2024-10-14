#include <Sel/Font.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fmt/core.h>
#include <cassert>
#include <stdexcept>

namespace Sel
{
	Font::Font(TTF_Font* font, std::string filepath) :
	Asset(std::move(filepath)),
	m_font(font)
	{
	}

	Font::Font(Font&& font) noexcept :
	Asset(std::move(font)),
	m_font(font.m_font)
	{
		font.m_font = nullptr;
	}

	Font::~Font()
	{
		if (m_font)
			TTF_CloseFont(m_font);
	}

	Surface Font::RenderUTF8Text(int characterSize, const std::string& str, const Color& color)
	{
		SDL_Color sdlColor;
		color.ToRGBA8(sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a);

		TTF_SetFontSize(m_font, characterSize);
		return Surface(TTF_RenderUTF8_Blended(m_font, str.data(), sdlColor), "");
	}

	Font& Font::operator=(Font&& font) noexcept
	{
		Asset::operator=(std::move(font));
		std::swap(m_font, font.m_font);
		return *this;
	}

	Font Font::OpenFromFile(const std::string& path)
	{
		TTF_Font* font = TTF_OpenFont(path.c_str(), 24);
		if (!font)
			throw std::runtime_error(SDL_GetError());

		return Font(font, path);
	}
}