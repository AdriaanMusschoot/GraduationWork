#include "GUI.h"

#include <imgui.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include "Scene.h"
#include "SdEngine.h"
#include "BVHNode.h"
#include "Misc.h"
#include "SDFObjects.h"

void GUI::Initialize(SDL_Window* windowPtr, SDL_Renderer* rendererPtr)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(windowPtr, rendererPtr);
    ImGui_ImplSDLRenderer2_Init(rendererPtr);
}

void GUI::BeginFrame(sdf::Engine& engine)
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

	static ImVec2 const settingsDimensions{ 150, 300 };
	static ImVec2 const settingsPosition{ 0, 0 };
	static ImVec2 const statsDimensions{ 150, 300 };
	static ImVec2 const statsPosition{ engine.GetRenderer().GetWindowDimensions().x - statsDimensions.x, 0 };

	LoadSettingsWindow(engine, "Settings", settingsPosition, settingsDimensions);
	LoadStatsWindow(engine, "Statistics", statsPosition, statsDimensions);
}

void GUI::EndFrame()
{
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Destroy()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

bool GUI::ProcessEvent(SDL_Event* eventPtr)
{
    return ImGui_ImplSDL2_ProcessEvent(eventPtr);;
}

void GUI::LoadSettingsWindow(sdf::Engine& engine, std::string const& name, ImVec2 const& pos, ImVec2 const& size)
{
    ImGui::Begin(name.c_str());
    ImGui::SetWindowPos(pos, ImGuiCond_Once);
    ImGui::SetWindowSize(size, ImGuiCond_Once);

    ImGui::Checkbox("Use Early Out", &sdf::Scene::m_UseEarlyOut);
	if (sdf::Scene::m_UseEarlyOut)
	{
        ImGui::Checkbox("Box Early Out", &sdf::Object::m_UseBoxEarlyOut);
	}
    else
    {
		sdf::Object::m_UseBoxEarlyOut = false;
    }

    ImGui::Checkbox("Use BVH", &sdf::Scene::m_UseBVH);
    if (sdf::Scene::m_UseBVH)
    {
        ImGui::Checkbox("Box BVH", &sdf::BVHNode::m_BoxBVH);
    }
	else
	{
		sdf::BVHNode::m_BoxBVH = false;
	}
	//ImGui::InputInt("BVH Stepss", &sdf::Scene::m_BVHSteps);

	ImGui::Text("Scene complexity: ");
    ImGui::Combo("|", &engine.SetCurrentSceneID(), engine.GetSceneComplexities(), engine.GetSceneComplexityCount());

    sdf::GameTimer& timer{ engine.GetTimer() };

    static int cameraPos{ 2 };
    ImGui::Text("CameraPos");
	if (ImGui::Button("Move Forward"))
	{
		sdf::Scene::MoveCameraPos(3.0f);
        --cameraPos;
	}
	if (ImGui::Button("Move Backward"))
	{
        ++cameraPos;
		sdf::Scene::MoveCameraPos(-3.0f);
	}
    

	ImGui::Text("Time Benchmark: ");
	ImGui::InputFloat("##", &timer.SetBenchmarkTargetFrames());

    if (ImGui::Button("Start BenchMark"))
	{
        sdf::Renderer const& renderer{ engine.GetRenderer() };

        sdf::ResultStats const hitStats{ renderer.GetCollisionStats(false) };
        sdf::ResultStats const missStats{ renderer.GetCollisionStats(true) };

        std::string imgName{ engine.GetSceneComplexities()[engine.SetCurrentSceneID()] };
        imgName += sdf::Scene::m_UseEarlyOut ? "1" : "0";
        imgName += sdf::Object::m_UseBoxEarlyOut ? "1" : "0";
        imgName += sdf::Scene::m_UseBVH ? "1" : "0";
        imgName += sdf::BVHNode::m_BoxBVH ? "1" : "0";
		imgName += std::to_string(cameraPos);

        if (renderer.SaveBufferToImage(imgName))
        {
            std::cout << "Benchmark image saved successfully" << "\n";
        }

		timer.StartBenchmark(engine.GetSceneComplexities()[engine.SetCurrentSceneID()], hitStats, missStats);
    }

    ImGui::End();
}

void GUI::LoadStatsWindow(sdf::Engine& engine, std::string const& name, ImVec2 const& pos, ImVec2 const& size)
{
	ImGui::Begin(name.c_str());
    ImGui::SetWindowPos(pos, ImGuiCond_Once);
    ImGui::SetWindowSize(size, ImGuiCond_Once);

    ImGui::Value("FPS", ImGui::GetIO().Framerate);
    
    sdf::Renderer const& renderer{ engine.GetRenderer() };
    
	sdf::ResultStats const hitStats{ renderer.GetCollisionStats(false) };
	sdf::ResultStats const missStats{ renderer.GetCollisionStats(true) };

    ImGui::Separator();
    ImGui::Text("Hit Statistics");
    ImGui::Text("Rays hit: %d", hitStats.Count);
	ImGui::Text("Avg steps: %d", hitStats.AverageStepsThroughScene);
	ImGui::Text("Avg early out: %d", hitStats.AverageEarlyOutSteps);
	ImGui::Text("Avg BVH depth: %d", hitStats.AverageBVHDepth);
    ImGui::Separator();
    ImGui::Text("Miss Statistics");
    ImGui::Text("Rays missed: %d", missStats.Count);
	ImGui::Text("Avg steps: %d", missStats.AverageStepsThroughScene);
	ImGui::Text("Avg early out: %d", missStats.AverageEarlyOutSteps);
	ImGui::Text("Avg BVH depth: %d", missStats.AverageBVHDepth);

    ImGui::End();
}
