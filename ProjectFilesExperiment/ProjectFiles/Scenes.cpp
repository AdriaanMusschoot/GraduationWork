#include "Scenes.h"

#include <numbers>
#include <random>
#include <array>

#include "SDFObjects.h"

sdf::SceneEasyComplexity::SceneEasyComplexity()
{
    constexpr float spacing{ 1.5f };
     
    //m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Sphere>(0.5f, glm::vec3{ 0.f, 0.f, 0.0f }, colors::Red));

    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Link>(0.8f, 0.8f, 0.2f, glm::vec3{ 0.f, 0.f, spacing }, colors::Red));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Link>(0.5f, 0.5f, 0.13f, glm::vec3{ -spacing, spacing, 0.f }, colors::Red));
    
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Octahedron>(1.f, glm::vec3{ spacing, spacing, spacing }, colors::Green));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Octahedron>(1.1f, glm::vec3{ -spacing, -spacing, spacing }, colors::Green));
    
	CreateBVHStructure();
}

sdf::SceneMediumComplexity::SceneMediumComplexity()
{
    constexpr float spacing{ 2.0f };
    constexpr float halfSpacing{ spacing / 2.0f };

    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::BoxFrame>(glm::vec3{ 0.7f, 0.7f, 0.7f }, 0.05f, glm::vec3{ 0.f, halfSpacing, 0.f }, colors::Blue));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::BoxFrame>(glm::vec3{ 0.6f, 0.6f, 0.6f }, 0.04f, glm::vec3{ -spacing, -halfSpacing, spacing }, colors::Blue));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::BoxFrame>(glm::vec3{ 0.8f, 0.8f, 0.8f }, 0.06f, glm::vec3{ spacing, -halfSpacing, -spacing }, colors::Blue));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::BoxFrame>(glm::vec3{ 0.5f, 0.5f, 0.5f }, 0.03f, glm::vec3{ -spacing, spacing, -spacing }, colors::Blue));
    
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::HexagonalPrism>(0.6f, 0.6f, glm::vec3{ spacing, -halfSpacing, spacing }, colors::Yellow));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::HexagonalPrism>(0.8f, 0.8f, glm::vec3{ -spacing, 0.f, -halfSpacing }, colors::Yellow));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::HexagonalPrism>(0.5f, 0.5f, glm::vec3{ spacing, halfSpacing, -spacing }, colors::Yellow));
    
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Pyramid>(2.f, glm::vec3{ 0.f, -spacing, 0.f }, colors::Magenta));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Pyramid>(1.8f, glm::vec3{ spacing, halfSpacing, halfSpacing }, colors::Magenta));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Pyramid>(2.2f, glm::vec3{ -spacing, -spacing, -spacing }, colors::Magenta));
    
    CreateBVHStructure();
}

sdf::SceneHighComplexity::SceneHighComplexity()
{
    constexpr float spacing{ 4.0f };
    constexpr float halfSpacing{ spacing / 2.0f };

    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ 0.f, spacing, 0.f }, colors::Cyan));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ halfSpacing, -halfSpacing, halfSpacing }, colors::Cyan));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ 0.f, -spacing, -halfSpacing }, colors::Yellow));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ 0.f, 0.f, spacing }, colors::Yellow));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ spacing, 0.f, 0.f }, colors::Magenta));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ -spacing, spacing, halfSpacing }, colors::Magenta));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ spacing, spacing, -spacing }, colors::Red));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ -spacing, 0.f, -halfSpacing }, colors::Red));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ -spacing, -spacing, 0.f }, colors::Green));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ -halfSpacing, 0.f, -spacing }, colors::Green));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ halfSpacing, -spacing, -spacing }, colors::Blue));
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ -halfSpacing, -halfSpacing, spacing }, colors::Blue));
    
    CreateBVHStructure();
}

sdf::SceneLink::SceneLink()
{
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Link>(0.8f, 0.8f, 0.2f, glm::vec3{ 0.f, 0.f, 0.f }, colors::Red));
}

sdf::SceneOctahedron::SceneOctahedron()
{
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Octahedron>(1.3f, glm::vec3{ 0.f, 0.f, 0.f }, colors::Green));
}

sdf::SceneBoxFrame::SceneBoxFrame()
{
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::BoxFrame>(glm::vec3{ 0.9f, 0.9f, 1.f }, 0.1f, glm::vec3{ 0.f, 0.f, 0.f }, colors::Blue));
}

sdf::SceneHexagonalPrism::SceneHexagonalPrism()
{
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::HexagonalPrism>(0.8f, 0.8f, glm::vec3{ 0.f, 0.f, 0.f }, colors::Yellow));
}

sdf::ScenePyramid::ScenePyramid()
{
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::Pyramid>(4.f, glm::vec3{ 0.f, -1.5f, 0.f }, colors::Magenta));
}

sdf::SceneMandelBulb::SceneMandelBulb()
{
    m_SDObjectUPtrVec.emplace_back(std::make_unique<sdf::MandelBulb>(glm::vec3{ 0.f, 0.f, 0.f }, colors::Blue));
}
