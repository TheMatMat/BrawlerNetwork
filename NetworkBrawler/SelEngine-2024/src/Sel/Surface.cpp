#include <Sel/Surface.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <fmt/core.h>
#include <cassert>
#include <stdexcept>

namespace Sel
{
	Surface::Surface(SDL_Surface* surface, std::string filepath) :
	Asset(std::move(filepath)),
	m_surface(surface)
	{
	}

	SDL_Surface* Surface::GetHandle() const
	{
		return m_surface;
	}

	Surface::Surface(Surface&& surface) noexcept :
	Asset(std::move(surface)),
	m_surface(surface.m_surface)
	{
		surface.m_surface = nullptr;
	}

	Surface::~Surface()
	{
		if (m_surface)
			SDL_FreeSurface(m_surface);
	}

	void Surface::FillRect(const SDL_Rect& rect, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
	{
		assert(m_surface);
		SDL_FillRect(m_surface, &rect, SDL_MapRGBA(m_surface->format, r, g, b, a));
	}

	SDL_Surface* Surface::GetHandle()
	{
		return m_surface;
	}

	std::uint8_t* Surface::GetPixels()
	{
		assert(m_surface);
		return static_cast<std::uint8_t*>(m_surface->pixels);
	}

	const std::uint8_t* Surface::GetPixels() const
	{
		assert(m_surface);
		return static_cast<const std::uint8_t*>(m_surface->pixels);
	}

	Surface& Surface::operator=(Surface&& surface) noexcept
	{
		Asset::operator=(std::move(surface));
		std::swap(m_surface, surface.m_surface);
		return *this;
	}

	Surface Surface::Create(int width, int height)
	{
		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
		if (!surface)
			throw std::runtime_error(SDL_GetError());

		return Surface(surface, {});
	}

	Surface Surface::LoadFromFile(const std::string& path)
	{
		SDL_Surface* surface = IMG_Load(path.c_str());
		if (!surface)
			throw std::runtime_error(SDL_GetError());

		return Surface(surface, path);
	}
}