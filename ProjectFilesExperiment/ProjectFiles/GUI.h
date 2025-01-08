#pragma once

struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

#include <string>
#include <imgui.h>

namespace sdf
{
	class Engine;
}

namespace GUI
{
	void Initialize(SDL_Window* windowPtr, SDL_Renderer* randererPtr);

	void BeginFrame(sdf::Engine& engine);
	void EndFrame();
	void Destroy();

	bool ProcessEvent(SDL_Event* eventPtr);

	void LoadSettingsWindow(sdf::Engine& engine, std::string const& name, ImVec2 const& pos, ImVec2 const& size);
	void LoadStatsWindow(sdf::Engine& engine, std::string const& name, ImVec2 const& pos, ImVec2 const& size);
}