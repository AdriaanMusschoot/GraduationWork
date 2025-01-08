#include "SdEngine.h"

#include <SDL.h>
#include <SDL_events.h>

#include "GUI.h"
#include "Scenes.h"

sdf::Engine::Engine(uint32_t const& width, uint32_t const& height)
    : m_Renderer{ width, height }
	, m_Timer{}
{
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneEasyComplexity>());
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneMediumComplexity>());
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneHighComplexity>());
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneLink>());
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneOctahedron>());
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneBoxFrame>());
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneHexagonalPrism>());
    m_SceneUPtrVec.emplace_back(std::make_unique<ScenePyramid>());
    m_SceneUPtrVec.emplace_back(std::make_unique<SceneMandelBulb>());
}

void sdf::Engine::Run()
{
    while (not ShouldQuit)
    {
        m_Timer.Update();

        GUI::BeginFrame(*this);
        
        HandleInput();

        m_SceneUPtrVec[m_CurrentSceneID]->Update(m_Timer.GetElapsed());
        
        m_Renderer.Render(*m_SceneUPtrVec[m_CurrentSceneID]);
    }
}

int& sdf::Engine::SetCurrentSceneID()
{
    return m_CurrentSceneID;
}

char const* const* sdf::Engine::GetSceneComplexities() const
{
    return m_SceneComplexity.data();
}

int sdf::Engine::GetSceneComplexityCount() const
{
    return m_SceneComplexity.size();
}

void sdf::Engine::HandleInput()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (GUI::ProcessEvent(&e))
            continue;

        switch (e.type)
        {
        case SDL_QUIT:
            ShouldQuit = true;
            break;
        default: ;
        }
    }
}
