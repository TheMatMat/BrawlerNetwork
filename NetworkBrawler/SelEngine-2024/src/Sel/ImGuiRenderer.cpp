#include <Sel/ImGuiRenderer.hpp>
#include <Sel/Renderer.hpp>
#include <Sel/Window.hpp>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

namespace Sel
{
	ImGuiRenderer::ImGuiRenderer(Window& window, Renderer& renderer)
	{
		// Setup imgui
		IMGUI_CHECKVERSION();
		m_context = ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForSDLRenderer(window.GetHandle(), renderer.GetHandle());
		ImGui_ImplSDLRenderer2_Init(renderer.GetHandle());
	}

	ImGuiRenderer::~ImGuiRenderer()
	{
		// Cleanup
		ImGui_ImplSDLRenderer2_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext(m_context);
	}

	ImGuiContext* ImGuiRenderer::GetContext()
	{
		return m_context;
	}

	void ImGuiRenderer::Render(Renderer& renderer)
	{
		ImGui::Render();
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer.GetHandle());
	}

	void ImGuiRenderer::NewFrame()
	{
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiRenderer::ProcessEvent(SDL_Event& event)
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
	}	
}