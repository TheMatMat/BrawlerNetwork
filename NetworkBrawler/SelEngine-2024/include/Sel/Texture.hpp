#pragma once

#include <Sel/Asset.hpp>
#include <SDL2/SDL.h>
#include <string>

struct SDL_Texture;

namespace Sel
{
	class Renderer;
	class Surface;

	class SEL_ENGINE_API Texture : public Asset
	{
		friend class Renderer;

		public:
			Texture(const Texture&) = delete;
			Texture(Texture&& texture) noexcept;
			~Texture();

			SDL_Texture* GetHandle();
			SDL_Rect GetRect() const;

			Texture& operator=(const Texture&) = delete;
			Texture& operator=(Texture&& texture) noexcept;

			static Texture CreateFromSurface(const Renderer& renderer, const Surface& surface);
			static Texture LoadFromFile(const Renderer& renderer, const std::string& filepath);

		private:
			explicit Texture(SDL_Texture* texture, std::string filepath);

			SDL_Texture* GetHandle() const;

			SDL_Texture* m_texture;
	};
}