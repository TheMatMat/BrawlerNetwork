#pragma once

#include <Sel/Export.hpp>

union SDL_Event;
struct ImGuiContext;

namespace Sel
{
	class Renderer;
	class Window;
		
	class SEL_ENGINE_API ImGuiRenderer
	{
		public:
			ImGuiRenderer(Window& window, Renderer& renderer);
			ImGuiRenderer(const ImGuiRenderer&) = delete;
			ImGuiRenderer(ImGuiRenderer&&) = delete;
			~ImGuiRenderer();

			ImGuiContext* GetContext();

			void Render(Renderer& renderer);

			void NewFrame();

			void ProcessEvent(SDL_Event& event);

			ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;
			ImGuiRenderer& operator=(ImGuiRenderer&&) = delete;

		private:
			ImGuiContext* m_context;
	};
}
