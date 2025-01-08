#include "SDFObjects.h"

#include <algorithm>
#include <execution>
#include <optional>
#include <array>

#include "Misc.h"
#include <iostream>

bool sdf::Object::m_UseBoxEarlyOut{ true };

sdf::Object::Object(glm::vec3 const& origin, sdf::ColorRGB const& color)
    : m_Origin{ origin }, m_Color{ color }
{
}

float sdf::Object::GetDistance(glm::vec3 const& point, bool useEarlyOuts, sdf::HitRecord& outHitRecord)
{
    if (useEarlyOuts)
    {
        float const earlyOutDistance{ EarlyOutTest(point) };
        if (earlyOutDistance >= 0.001f)
        {
			++outHitRecord.EarlyOutUsage;
            return earlyOutDistance;
        }
    }
    return GetDistanceUnoptimized(point);
}

float sdf::Object::EarlyOutTest(glm::vec3 const& point)
{
    if (m_UseBoxEarlyOut)
    {
        glm::vec3 const q{ glm::abs(point) - m_BoxExtent };
        return glm::length(glm::max(q, 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
    }
    return glm::length(point) - m_EarlyOutRadius;
}

void sdf::Object::FurthestSurfaceConcentricCircles(float initialRadius)
{
    float radius{ initialRadius };

    std::mutex closestPointMutex{};
    std::optional<glm::vec3> surfacePoint{};
    //recursion bad cause stack overflow use while
    do
    {
        std::vector<glm::vec3> points{ GenerateSpherePoints(glm::vec3{ 0.f, 0.f, 0.f }, radius) };

        std::vector<float> closestDistanceVec(std::thread::hardware_concurrency(), std::numeric_limits<float>::max());
        std::vector<glm::vec3> closestPointVec(std::thread::hardware_concurrency(), glm::vec3{ 0.f, 0.f, 0.f });

        std::for_each(std::execution::par_unseq, points.begin(), points.end(),
            [&](const glm::vec3& point)
            {
                float distance{ GetDistanceUnoptimized(point) };

                if (distance < 0.001f)
                {
                    std::lock_guard<std::mutex> lock(closestPointMutex);
                    if (not surfacePoint.has_value())
                    {
                        surfacePoint = point;
                    }
                    return;
                }

                size_t threadId{ std::hash<std::thread::id>{}(std::this_thread::get_id()) % closestDistanceVec.size() };
                if (auto& closestDistance{ closestDistanceVec[threadId] };
                    distance < closestDistance)
                {
                    closestDistance = distance;
                    closestPointVec[threadId] = point;
                }
            });
        auto minElementIter = std::min_element(closestDistanceVec.begin(), closestDistanceVec.end());
        size_t minElementIndex = std::distance(closestDistanceVec.begin(), minElementIter);

        radius = glm::length(closestPointVec[minElementIndex]) - *minElementIter;

    } while (not surfacePoint.has_value());

    m_EarlyOutRadius = glm::length(surfacePoint.value());
}

void sdf::Object::FurthestSurfaceAlongAxis(float initialDistance)
{
	std::mutex closestPointMutex{};

    static std::array<std::pair<int, glm::vec3>, 6> directionArr
    {
        std::make_pair(0, glm::vec3{1, 0, 0}), std::make_pair(1, glm::vec3{-1, 0, 0 }),
        std::make_pair(2, glm::vec3{0, 1, 0}), std::make_pair(3, glm::vec3{0, -1, 0 }),
        std::make_pair(4, glm::vec3{0, 0, 1}), std::make_pair(5, glm::vec3{0, 0, -1 })
    };

    std::array<float, 6> distanceArr{};
	std::fill(std::execution::par_unseq, distanceArr.begin(), distanceArr.end(), initialDistance);
    std::array<std::optional<glm::vec3>, 6> pointArr{};

    do
    {
        std::for_each(std::execution::par_unseq, directionArr.begin(), directionArr.end(),
            [&](std::pair<int, glm::vec3> const& direction)
            {
                {
					std::lock_guard<std::mutex> lock(closestPointMutex);
				    if (pointArr[direction.first].has_value())
				    {
				    	return;
				    }
                }
                std::vector<glm::vec3> points{ GenerateWallPoints(direction.second, 0.01, distanceArr[direction.first]) };

                std::vector<float> closestDistanceVec(std::thread::hardware_concurrency(), std::numeric_limits<float>::max());
                std::vector<glm::vec3> closestPointVec(std::thread::hardware_concurrency(), glm::vec3{ 0.f, 0.f, 0.f });

                std::for_each(std::execution::par_unseq, points.begin(), points.end(),
                    [&](const glm::vec3& point)
                    {
                        float distance{ GetDistanceUnoptimized(point) };

                        if (distance < 0.1f)
                        {
                            std::lock_guard<std::mutex> lock(closestPointMutex);
                            if (not pointArr[direction.first].has_value())
                            {
                                pointArr[direction.first] = point;
                            }
                            return;
                        }

                        size_t threadId{ std::hash<std::thread::id>{}(std::this_thread::get_id()) % closestDistanceVec.size() };
                        if (auto& closestDistance{ closestDistanceVec[threadId] };
                            distance < closestDistance)
                        {
                            closestDistance = distance;
                            closestPointVec[threadId] = point;
                        }
                    });
                auto minElementIter = std::min_element(closestDistanceVec.begin(), closestDistanceVec.end());
                size_t minElementIndex = std::distance(closestDistanceVec.begin(), minElementIter);

                float const newDistanceValue{ glm::length(closestPointVec[minElementIndex] * direction.second) - *minElementIter };

                distanceArr[direction.first] = newDistanceValue;
			});
    } 
    while 
        (
            std::any_of(std::execution::par_unseq, pointArr.begin(), pointArr.end(),
            [](std::optional<glm::vec3> const& point)
            {
                return not point.has_value();
            })
        );

	std::for_each(std::execution::par_unseq, pointArr.begin(), pointArr.end(),
		[&](std::optional<glm::vec3> const& point)
		{
			if (float const positiveXValue{ glm::abs(point.value().x) };
                m_BoxExtent.x < positiveXValue)
			{
				m_BoxExtent.x = positiveXValue;
			}
			if (float const positiveYValue{ glm::abs(point.value().y) };
                m_BoxExtent.y < positiveYValue)
            {
                m_BoxExtent.y = positiveYValue;
			}
			if (float const positiveZValue{ glm::abs(point.value().z) };
                m_BoxExtent.z < positiveZValue)
            {
                m_BoxExtent.z = positiveZValue;
			}
		});
}

glm::vec3 const& sdf::Object::Origin() const
{
    return m_Origin;
}

sdf::ColorRGB const& sdf::Object::Shade() const
{
    return m_Color;
}

float sdf::Object::GetEarlyOutRadius() const
{
    return m_EarlyOutRadius;
}

sdf::Sphere::Sphere(float radius, glm::vec3 const& origin, sdf::ColorRGB const& color)
    : Object(origin, color)
    , m_Radius{ radius }
{
}

float sdf::Sphere::GetDistanceUnoptimized(glm::vec3 const& point)
{
    return glm::length(point) - m_Radius;
}

sdf::Link::Link(float height, float innerRadius, float tubeRadius, glm::vec3 const& origin, sdf::ColorRGB const& color)
    : Object(origin, color)
    , m_HeightEmptySpace{ height }
    , m_InnerRadius{ innerRadius }
    , m_RadiusTube{ tubeRadius }
{
	FurthestSurfaceAlongAxis();
    FurthestSurfaceConcentricCircles();
}

float sdf::Link::GetDistanceUnoptimized(glm::vec3 const& point)
{
    glm::vec3 const p{ point };

    glm::vec3 const q{ p.x, glm::max(glm::abs(p.y) - m_HeightEmptySpace, 0.0f), p.z };
    glm::vec2 const qxy{ q };
    return glm::length(glm::vec2{ glm::length(qxy) - m_InnerRadius, q.z }) - m_RadiusTube;
}

sdf::Octahedron::Octahedron(float size, glm::vec3 const& origin, sdf::ColorRGB const& color)
    : Object(origin, color)
    , m_Size{ size }
{
	FurthestSurfaceAlongAxis();
    FurthestSurfaceConcentricCircles();
}

float sdf::Octahedron::GetDistanceUnoptimized(glm::vec3 const& point)
{
    glm::vec3 p{ glm::abs(point) };
    float m{ p.x + p.y + p.z - m_Size };

    glm::vec3 q;
    if (3.0f * p.x < m)
    {
        q = p;
    }
    else if (3.0f * p.y < m)
    {
        q.x = p.y;
        q.y = p.z;
        q.z = p.x;
    }
    else if (3.0f * p.z < m)
    {
        q.x = p.z;
        q.y = p.x;
        q.z = p.y;
    }
    else
    {
        return m * 0.57735027f;
    }

    float k{ glm::clamp(0.5f * (q.z - q.y + m_Size), 0.0f, m_Size) };
    return glm::length(glm::vec3{ q.x, q.y - m_Size + k, q.z - k });
}

sdf::BoxFrame::BoxFrame(glm::vec3 const& boxExtent, float roundedValue, glm::vec3 const& origin, sdf::ColorRGB const& color)
    : Object(origin, color)
    , m_BoxExtent{ boxExtent }
    , m_RoundedValue{ roundedValue }
{
	FurthestSurfaceAlongAxis();
    FurthestSurfaceConcentricCircles();
}

float sdf::BoxFrame::GetDistanceUnoptimized(glm::vec3 const& point)
{
    glm::vec3 const p{ abs(point) - m_BoxExtent };
    glm::vec3 const q{ abs(p + m_RoundedValue) - m_RoundedValue };
    return glm::min(glm::min(
        glm::length(glm::max(glm::vec3(p.x, q.y, q.z), 0.0f)) + glm::min(glm::max(p.x, glm::max(q.y, q.z)), 0.0f),
        glm::length(glm::max(glm::vec3(q.x, p.y, q.z), 0.0f)) + glm::min(glm::max(q.x, glm::max(p.y, q.z)), 0.0f)),
        glm::length(glm::max(glm::vec3(q.x, q.y, p.z), 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, p.z)), 0.0f));;
}

sdf::HexagonalPrism::HexagonalPrism(float depth, float radius, glm::vec3 const& origin, sdf::ColorRGB const& color)
    : Object(origin, color)
    , m_Depth{ depth }
    , m_Radius{ radius }
{
	FurthestSurfaceAlongAxis();
    FurthestSurfaceConcentricCircles();
}

float sdf::HexagonalPrism::GetDistanceUnoptimized(glm::vec3 const& point)
{
    static glm::vec3 const k{ -0.8660254f, 0.5f, 0.57735f };
    static glm::vec2 const kxy{ k };

    glm::vec3 p{ point };

    p = glm::abs(p);
    glm::vec2 const pxy{ p };

    glm::vec2 const subtVal{ 2.0f * glm::min(glm::dot(kxy, pxy), 0.0f) * kxy };

    p.x -= subtVal.x;
    p.y -= subtVal.y;

    glm::vec2 d
    {
        glm::length((glm::vec2{ p } - glm::vec2{glm::clamp(p.x, -k.z * m_Radius, k.z * m_Radius), m_Radius}) * glm::sign(p.y - m_Radius)),
        p.z - m_Depth
    };

    return glm::min(glm::max(d.x, d.y), 0.f) + glm::length(glm::max(d, 0.0f));
}

sdf::Pyramid::Pyramid(float height, glm::vec3 const& origin, sdf::ColorRGB const& color)
    : Object(origin, color)
    , m_Height{ height }
{
	FurthestSurfaceAlongAxis();
    FurthestSurfaceConcentricCircles();
}

float sdf::Pyramid::GetDistanceUnoptimized(glm::vec3 const& point)
{
    glm::vec3 p{ point };

    float const m2{ m_Height * m_Height + 0.25f };

    p.x = glm::abs(p.x);
    p.z = glm::abs(p.z);

    if (p.z > p.x)
    {
        glm::vec3 const tempPoint{ p };
        p.x = tempPoint.z;
        p.z = tempPoint.x;
    }
    p.x -= 0.5f;
    p.z -= 0.5f;

    glm::vec3 const q{ p.z, m_Height * p.y - 0.5f * p.x, m_Height * p.x + 0.5f * p.y };

    float const s{ glm::max(-q.x, 0.0f) };
    float const t{ glm::clamp((q.y - 0.5f * p.z) / (m2 + 0.25f), 0.0f, 1.0f) };

    float const a{ m2 * (q.x + s) * (q.x + s) + q.y * q.y };
    float const b{ m2 * (q.x + 0.5f * t) * (q.x + 0.5f * t) + (q.y - m2 * t) * (q.y - m2 * t) };

    float const d2{ glm::min(q.y, -q.x * m2 - q.y * 0.5f) > 0.0f ? 0.0f : glm::min(a, b) };

    return glm::sqrt((d2 + q.z * q.z) / m2) * glm::sign(glm::max(q.z, -p.y));
}

sdf::MandelBulb::MandelBulb(glm::vec3 const& origin, sdf::ColorRGB const& color)
    : Object(origin, color)
{
	FurthestSurfaceAlongAxis();
    FurthestSurfaceConcentricCircles();
}

float sdf::MandelBulb::GetDistanceUnoptimized(glm::vec3 const& point)
{
    glm::vec3 const p{ point };

    int iterations = 10;
    glm::vec3 w = p;
    float m = glm::dot(w, w);
    glm::vec4 trap{ abs(w), m };
    float dz = 1;

    for (int i{ 0 }; i < iterations; i++)
    {
        float m2 = m * m;
        float m4 = m2 * m2;
        dz = 8 * glm::sqrt(m4 * m2 * m) * dz + 1.0;

        float x = w.x; float x2 = x * x; float x4 = x2 * x2;
        float y = w.y; float y2 = y * y; float y4 = y2 * y2;
        float z = w.z; float z2 = z * z; float z4 = z2 * z2;

        float k3 = x2 + z2;
        float k2 = glm::inversesqrt(k3 * k3 * k3 * k3 * k3 * k3 * k3);
        float k1 = x4 + y4 + z4 - 6.0 * y2 * z2 - 6.0 * x2 * y2 + 2.0 * z2 * x2;
        float k4 = x2 - y2 + z2;
        w.x = p.x + 64.0 * x * y * z * (x2 - z2) * k4 * (x4 - 6.0 * x2 * z2 + z4) * k1 * k2;
        w.y = p.y + -16.0 * y2 * k3 * k4 * k4 + k1 * k1;
        w.z = p.z + -8.0 * y * k4 * (x4 * x4 - 28.0 * x4 * x2 * z2 + 70.0 * x4 * z4 - 28.0 * x2 * z2 * z4
            + z4 * z4) * k1 * k2;

        trap = glm::min(trap, glm::vec4{ abs(w), m });

        m = dot(w, w);
        if (m > 256)
            break;
    }
    return 0.25f * glm::log(m) * sqrt(m) / dz * m_Radius;
}

float sdf::SmoothMin(float dist1, float dist2, float smoothness)
{
    float h{ glm::max(smoothness - glm::abs(dist1 - dist2), 0.0f) / smoothness };
    constexpr float smoothFraction{ 1.0f / 6.0f };
    return glm::min(dist1, dist2) - h * h * h * smoothness * smoothFraction;
}

std::vector<glm::vec3> sdf::GenerateSpherePoints(glm::vec3 const& origin, float radius)
{
    static std::array<uint32_t, sdf::PointCountSphereHorizontal* sdf::PointCountSphereVertical> idxPointArr
    {
        []()
        {
            std::iota(idxPointArr.begin(), idxPointArr.end(), 0);
            return idxPointArr;
        }()
    };

    std::vector<glm::vec3> spherePointVec{ idxPointArr.size() };

    std::for_each(std::execution::par_unseq, idxPointArr.begin(), idxPointArr.end(),
        [&](uint32_t index)
        {
            int i = index % sdf::PointCountSphereHorizontal;
            int j = index / sdf::PointCountSphereHorizontal;

            float theta = glm::radians(static_cast<float>(i));
            float phi = glm::radians(static_cast<float>(j));

            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);

            spherePointVec[index] = origin + glm::vec3(x, y, z);
        });

    return spherePointVec;
}

std::vector<glm::vec3> sdf::GenerateWallPoints(glm::vec3 const& direction, float intervalDistance, float distance)
{
    static std::array<uint32_t, sdf::PointCountWall* sdf::PointCountWall> idxPointArr
    {
        []()
        {
			std::iota(idxPointArr.begin(), idxPointArr.end(), 0);
            return idxPointArr;
        }()
    };

    std::vector<glm::vec3> wallPointVec{ idxPointArr.size() };

    std::for_each(std::execution::par_unseq, idxPointArr.begin(), idxPointArr.end(),
        [&](uint32_t index)
        {
            int const i{ static_cast<int>(index % sdf::PointCountWall) };
            int const j{ static_cast<int>(index / sdf::PointCountWall) };

            int const normalized_i{ i - sdf::PointCountWall / 2 };
            int const normalized_j{ j - sdf::PointCountWall / 2 };  

            wallPointVec[index] =
                [&]()
                {
                    // Scale the normalized values to the desired distance and radius
                    if (direction.x != 0)
                    {
                        return glm::vec3{ direction.x * distance, normalized_i * intervalDistance, normalized_j * intervalDistance };
                    }
                    else if (direction.y != 0)
                    {
                        return glm::vec3{ normalized_j * intervalDistance, direction.y * distance, normalized_i * intervalDistance };
                    }
                    else
                    {
                        return glm::vec3{ normalized_i * intervalDistance, normalized_j * intervalDistance, direction.z * distance };
                    }
                }();
        });


    return wallPointVec;
}

