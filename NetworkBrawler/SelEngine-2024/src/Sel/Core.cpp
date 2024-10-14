#include <Sel/Core.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>

namespace Sel
{
	Core::Core(std::uint32_t flags)
	{
		if (SDL_Init(flags) != 0)
			throw std::runtime_error(SDL_GetError());

		if (TTF_Init() != 0)
		{
			SDL_Quit();
			throw std::runtime_error(SDL_GetError());
		}
	}

	Core::~Core()
	{
		TTF_Quit();
		SDL_Quit();
	}

	Vector2i Core::GetMousePosition()
	{
		Vector2i mousePos;
		SDL_GetMouseState(&mousePos.x, &mousePos.y);

		return mousePos;
	}

	bool Core::PollEvent(SDL_Event& event)
	{
		return SDL_PollEvent(&event) > 0;
	}
}
