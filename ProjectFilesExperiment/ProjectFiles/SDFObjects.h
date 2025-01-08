#pragma once
#include "glm/glm.hpp"

#include <vector>

#include "ColorRGB.h"

namespace sdf
{
    struct HitRecord;

    class Object
    {
    public:
        Object(glm::vec3 const& origin, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~Object() = default;

        float GetDistance(glm::vec3 const& point, bool useEarlyOuts, HitRecord& outHitRecord);

        glm::vec3 const& Origin() const;
        ColorRGB const& Shade() const;

        float GetEarlyOutRadius() const;

        static bool m_UseBoxEarlyOut;
    protected:
        virtual float GetDistanceUnoptimized(glm::vec3 const& point) = 0;

        void FurthestSurfaceConcentricCircles(float initialRadius = 10);
		void FurthestSurfaceAlongAxis(float initialDistance = 10);
    private:
        glm::vec3 m_Origin{ 0.f, 0.f, 0.f };
        float m_EarlyOutRadius{};

        glm::vec3 m_BoxExtent{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

        ColorRGB m_Color{ 1.f, 0.f, 0.f };

        float EarlyOutTest(glm::vec3 const& point);
    };

    class Sphere final : public Object
    {
    public:
        Sphere(float radius = 0.3f, glm::vec3 const& origin = glm::vec3{ 0.0f, 0.f, 0.f }, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~Sphere() = default;

        float GetDistanceUnoptimized(glm::vec3 const& point) override;
    private:
        float m_Radius{};
    };

    class Link final : public Object
    {
    public:
        Link(float height = 0.2f, float innerRadius = 0.2f, float tubeRadius = 0.07f, glm::vec3 const& origin = glm::vec3{ 1.f, 0.f, 0.f }, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~Link() = default;

        float GetDistanceUnoptimized(glm::vec3 const& point) override;
    private:
        float m_HeightEmptySpace{ 0.2f };
        float m_InnerRadius{ 0.2f };
        float m_RadiusTube{ 0.05f };
    };

    class Octahedron final : public Object
    {
    public:
        Octahedron(float size = 0.3f, glm::vec3 const& origin = glm::vec3{ -1.f, 0.f, 0.f }, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~Octahedron() = default;

        float GetDistanceUnoptimized(glm::vec3 const& point) override;
    private:
        float m_Size{ 0.3f };
    };

    class BoxFrame final : public Object
    {
    public:
        BoxFrame(glm::vec3 const& boxExtent = glm::vec3{ 0.3f, 0.3f, 0.3f }, float roundedValue = 0.02f, glm::vec3 const& origin = glm::vec3{ 1.f, 0.f, 0.f }, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~BoxFrame() = default;

        float GetDistanceUnoptimized(glm::vec3 const& point) override;
    private:
        glm::vec3 m_BoxExtent{};
        float m_RoundedValue{};
    };

    class HexagonalPrism final : public Object
    {
    public:
        HexagonalPrism(float depth = 0.2f, float radius = 0.3f, glm::vec3 const& origin = glm::vec3{ -1.f, 0.f, 0.f }, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~HexagonalPrism() = default;

        float GetDistanceUnoptimized(glm::vec3 const& point) override;
    private:
        float m_Depth{ 0.2f };
        float m_Radius{ 0.3f };
    };

    class Pyramid final : public Object
    {
    public:
        Pyramid(float height = 1.f, glm::vec3 const& origin = glm::vec3{ -0.f, 0.f, 0.f }, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~Pyramid() = default;

        float GetDistanceUnoptimized(glm::vec3 const& point) override;
    private:
        float m_Height{ 1.f };
    };

    class MandelBulb final : public Object
    {
    public:
        MandelBulb(glm::vec3 const& origin = glm::vec3{ 0.f, 0.f, -0.f }, ColorRGB const& color = ColorRGB{ 1.f, 0.f, 0.f });
        virtual ~MandelBulb() = default;

        float GetDistanceUnoptimized(glm::vec3 const& point) override;
    private:
        float m_Radius{ 1.0f };
    };

    static float SmoothMin(float dist1, float dist2, float smoothness);

    constexpr int PointCountSphereHorizontal{ 360 };
    constexpr int PointCountSphereVertical{ 360 };
    constexpr int PointCountWall{ 1001 };
    std::vector<glm::vec3> GenerateSpherePoints(glm::vec3 const& origin, float radius);
    std::vector<glm::vec3> GenerateWallPoints(glm::vec3 const& direction, float intervalDistance, float distance);
}