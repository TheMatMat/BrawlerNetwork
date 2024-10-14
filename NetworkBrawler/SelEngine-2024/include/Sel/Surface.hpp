#pragma once

#include <Sel/Asset.hpp>
#include <cstdint>
#include <string>

struct SDL_Surface;
struct SDL_Rect;

namespace Sel
{
	class SEL_ENGINE_API Surface : public Asset
	{
		friend class Font;
		friend class Texture;

		public:
			Surface(const Surface&) = delete;
			Surface(Surface&& surface) noexcept;
			~Surface();

			void FillRect(const SDL_Rect& rect, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);

			SDL_Surface* GetHandle();
			std::uint8_t* GetPixels();
			const std::uint8_t* GetPixels() const;

			Surface& operator=(const Surface&) = delete;
			Surface& operator=(Surface&& surface) noexcept;

			static Surface Create(int width, int height);
			static Surface LoadFromFile(const std::string& path);

		private:
			explicit Surface(SDL_Surface* surface, std::string filepath);

			SDL_Surface* GetHandle() const;

			SDL_Surface* m_surface;
	};
}