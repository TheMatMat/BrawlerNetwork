#pragma once

#include <Sel/Export.hpp>
#include <Sel/Vector2.hpp>
#include <SDL2/SDL.h>
#include <cstdint>

namespace Sel
{
	class SEL_ENGINE_API Core
	{
		public:
			Core(std::uint32_t flags = 0);
			Core(const Core&) = delete;
			~Core();

			Core& operator=(const Core&) = delete;

			static Vector2i GetMousePosition();
			static bool PollEvent(SDL_Event& event);
	};
}
