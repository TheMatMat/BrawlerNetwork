#pragma once

#include <Sel/Export.hpp>
#include <SDL2/SDL.h>
#include <cstddef>
#include <cstdint>

struct SDL_Renderer;

namespace Sel
{
	class Texture;
	class Window;

	class SEL_ENGINE_API Renderer
	{
		friend class Texture;

		public:
			Renderer(Window& window, int renderer = -1, std::uint32_t flags = 0);
			Renderer(const Renderer&) = delete;
			~Renderer();

			SDL_Renderer* GetHandle();

			void Clear();

			void RenderCopy(const Texture& texture);
			void RenderCopy(const Texture& texture, const SDL_Rect& destRect);
			void RenderCopy(const Texture& texture, const SDL_Rect& srcRect, const SDL_Rect& destRect);
			void RenderLines(const SDL_FPoint* points, std::size_t count);
			void RenderGeometry(const SDL_Vertex* vertices, int numVertices);
			void RenderGeometry(const Texture& texture, const SDL_Vertex* vertices, int numVertices);
			void RenderGeometry(const SDL_Vertex* vertices, int numVertices, const int* indices, int numIndices);
			void RenderGeometry(const Texture& texture, const SDL_Vertex* vertices, int numVertices, const int* indices, int numIndices);

			void Present();

			void SetDrawColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255);

			Renderer& operator=(const Renderer&) = delete;

		private:
			SDL_Renderer* GetHandle() const;

			SDL_Renderer* m_renderer;
	};
}
