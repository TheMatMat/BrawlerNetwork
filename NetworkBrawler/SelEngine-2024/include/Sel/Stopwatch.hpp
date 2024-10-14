#pragma once

#include <Sel/Export.hpp>
#include <cstdint>

namespace Sel
{
	class SEL_ENGINE_API Stopwatch
	{
		public:
			Stopwatch();

			float GetElapsedTime() const;

			float Restart();

		private:
			static float GetElapsedTime(std::uint64_t now, std::uint64_t lastTime);

			std::uint64_t m_lastTime;
	};
}