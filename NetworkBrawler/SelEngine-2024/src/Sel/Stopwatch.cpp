#include <Sel/Stopwatch.hpp>
#include <SDL2/SDL.h>

namespace Sel
{
	Stopwatch::Stopwatch() :
	m_lastTime(SDL_GetPerformanceCounter())
	{
	}

	float Stopwatch::GetElapsedTime() const
	{
		return GetElapsedTime(SDL_GetPerformanceCounter(), m_lastTime);
	}

	float Stopwatch::Restart()
	{
		Uint64 now = SDL_GetPerformanceCounter();
		float elapsedTime = GetElapsedTime(now, m_lastTime);
		m_lastTime = now;

		return elapsedTime;
	}

	float Stopwatch::GetElapsedTime(std::uint64_t now, std::uint64_t lastTime)
	{
		return static_cast<float>(now - lastTime) / SDL_GetPerformanceFrequency();
	}
}
