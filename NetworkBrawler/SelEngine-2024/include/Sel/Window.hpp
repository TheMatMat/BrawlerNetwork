#pragma once

#include <Sel/Export.hpp>
#include <Sel/Vector2.hpp>
#include <cstdint>
#include <string>

struct SDL_SysWMinfo;
struct SDL_Window;

namespace Sel
{
	class SEL_ENGINE_API Window
	{
		friend class Renderer;

		public:
			Window(const std::string& title, int width, int height, std::uint32_t flags = 0);
			Window(const std::string& title, int x, int y, int width, int height, std::uint32_t flags = 0);
			Window(const Window&) = delete;
			~Window();

			SDL_Window* GetHandle();
			Vector2i GetSize() const;
			bool GetWindowManagerInfo(SDL_SysWMinfo& sysInfo);
			std::string GetTitle() const;

			Window& operator=(const Window&) = delete;

		private:
			SDL_Window* m_window;
	};
}

