#include <Sel/Texture.hpp>
#include <Sel/Renderer.hpp>
#include <Sel/Surface.hpp>
#include <SDL2/SDL.h>
#include <stdexcept>

namespace Sel
{
	Texture::Texture(SDL_Texture* texture, std::string filepath) :
	Asset(std::move(filepath)),
	m_texture(texture)
	{
	}

	SDL_Texture* Texture::GetHandle() const
	{
		return m_texture;
	}

	Texture::Texture(Texture&& texture) noexcept :
	Asset(std::move(texture)),
	m_texture(texture.m_texture)
	{
		texture.m_texture = nullptr;
	}

	Texture::~Texture()
	{
		if (m_texture)
			SDL_DestroyTexture(m_texture);
	}

	SDL_Texture* Texture::GetHandle()
	{
		return m_texture;
	}

	SDL_Rect Texture::GetRect() const
	{
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		SDL_QueryTexture(m_texture, nullptr, nullptr, &rect.w, &rect.h);

		return rect;
	}

	Texture& Texture::operator=(Texture&& texture) noexcept
	{
		Asset::operator=(std::move(texture));
		std::swap(m_texture, texture.m_texture);
		return *this;
	}

	Texture Texture::CreateFromSurface(const Renderer& renderer, const Surface& surface)
	{
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer.GetHandle(), surface.GetHandle());
		if (!texture)
			throw std::runtime_error("failed to create texture");
	
		return Texture(texture, surface.GetFilepath());
	}

	Texture Texture::LoadFromFile(const Renderer& renderer, const std::string& filepath)
	{
		return CreateFromSurface(renderer, Surface::LoadFromFile(filepath));
	}
}