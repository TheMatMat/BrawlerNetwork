#include <Sel/Window.hpp>
#include <SDL2/SDL.h>
#include <stdexcept>
#include <SDL_syswm.h>

namespace Sel
{
	Window::Window(const std::string& title, int width, int height, std::uint32_t flags /* = 0 */) :
	Window(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags)
	{
	}

	Window::Window(const std::string& title, int x, int y, int width, int height, std::uint32_t flags /* = 0 */)
	{
		m_window = SDL_CreateWindow(title.c_str(), x, y, width, height, flags);
		if (!m_window)
			throw std::runtime_error("failed to create window");
	}

	Window::~Window()
	{
		SDL_DestroyWindow(m_window);
	}

	SDL_Window* Window::GetHandle()
	{
		return m_window;
	}

	Vector2i Window::GetSize() const
	{
		Vector2i size;
		SDL_GetWindowSize(m_window, &size.x, &size.y);

		return size;
	}

	bool Window::GetWindowManagerInfo(SDL_SysWMinfo& sysInfo)
	{
		SDL_VERSION(&sysInfo.version);
		return SDL_GetWindowWMInfo(m_window, &sysInfo) == SDL_TRUE;
	}

	std::string Window::GetTitle() const
	{
		return SDL_GetWindowTitle(m_window);
	}
}