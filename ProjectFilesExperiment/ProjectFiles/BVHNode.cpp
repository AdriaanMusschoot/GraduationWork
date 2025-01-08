#include "BVHNode.h"

#include <numeric>
#include <iostream>

#include "SDFObjects.h"
#include "Misc.h"

bool sdf::BVHNode::m_BoxBVH{ true };

sdf::BVHNode::BVHNode(glm::vec3 const& origin, float radius, glm::vec3 const& extent)
	: m_Origin{ origin }
	, m_Radius{ radius }
	, m_Extent{ extent }
{
}

std::pair<float, sdf::Object*> sdf::BVHNode::GetDistance(const glm::vec3& point, bool useEarlyOuts, HitRecord& outHitRecord) const
{
	//if (outHitRecord.BVHDepth >= maxSteps)
	//{
	//	if (m_ObjectUPtr)
	//	{
	//		return{ m_ObjectUPtr->GetDistance(point - m_ObjectUPtr->Origin(), useEarlyOuts, outHitRecord), m_ObjectUPtr };
	//	}
	//
	//	float const boundingVolumeDistance
	//	{
	//		[&]()
	//		{
	//			if (m_BoxBVH)
	//			{
	//				glm::vec3 const q{ glm::abs(point - m_Origin) - m_Extent };
	//				return glm::length(glm::max(q, 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
	//			}
	//			return glm::length(point - m_Origin) - m_Radius;
	//		}()
	//	};
	//	return { boundingVolumeDistance, nullptr };
	//}
	
	//if leaf node just return the distance to the object
	if (m_ObjectUPtr)
	{
		return { m_ObjectUPtr->GetDistance(point - m_ObjectUPtr->Origin(), useEarlyOuts, outHitRecord), m_ObjectUPtr };
	}
	++outHitRecord.BVHDepth;

	float const boundingVolumeDistance
	{ 
		[&]()
		{
			if (m_BoxBVH)
			{
				glm::vec3 const q{ glm::abs(point - m_Origin) - m_Extent };
				return glm::length(glm::max(q, 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
			}
			return glm::length(point - m_Origin) - m_Radius;
		}()
	};

	//if the distance to the bounding volume is large enough return it 
	if (boundingVolumeDistance > 0.1f)
	{
		return { boundingVolumeDistance, nullptr };
	}

	//we are in the bounding volume check the children
	auto const leftResult{ m_LeftNodeUPtr->GetDistance(point, useEarlyOuts, outHitRecord) };
	auto const rightResult{ m_RightNodeUPtr->GetDistance(point, useEarlyOuts, outHitRecord) };

	if (leftResult.first < rightResult.first)
	{
		return leftResult;
	}
	return rightResult;
}

std::unique_ptr<sdf::BVHNode> sdf::BVHNode::CreateBVHNode(std::vector<sdf::Object*> const& objects)
{
	if (objects.empty())
	{
		return nullptr;
	}

	glm::vec3 const origin{ CalculateBVHOrigin(objects) };
	float const radius{ CalculateBVHRadius(objects, origin) };
	glm::vec3 const extent{ CalculateBVHExtent(objects, origin) };

	std::unique_ptr nodeUPtr{ std::make_unique<BVHNode>(origin, radius, extent) };

	if (objects.size() == 1)
	{
		nodeUPtr->m_ObjectUPtr = objects[0];
	}
	else
	{
		auto [leftObjects, rightObjects] { std::move(SplitObjects(objects)) };
		nodeUPtr->m_LeftNodeUPtr = std::move(CreateBVHNode(leftObjects));
		nodeUPtr->m_RightNodeUPtr = std::move(CreateBVHNode(rightObjects));
	}

	std::cout << "BVHNode created with " << objects.size() << " objects" << std::endl;

	return std::move(nodeUPtr);
}

glm::vec3 sdf::BVHNode::CalculateBVHOrigin(std::vector<sdf::Object*> const& objects)
{
	glm::vec3 const origin
	{
		std::accumulate(objects.begin(), objects.end(), glm::vec3{ 0.f, 0.f, 0.f },
		[](glm::vec3 const& sum, sdf::Object const* obj)
		{
			return sum + obj->Origin();
		})
	};

	return origin / static_cast<float>(objects.size());
}

float sdf::BVHNode::CalculateBVHRadius(std::vector<sdf::Object*> const& objects, glm::vec3 const& origin)
{
	auto maxDistanceIt
	{
		std::max_element(objects.begin(), objects.end(),
		[&origin](sdf::Object const* a, sdf::Object const* b)
		{
			float distanceA = glm::length(a->Origin() - origin) + a->GetEarlyOutRadius();
			float distanceB = glm::length(b->Origin() - origin) + b->GetEarlyOutRadius();
			return distanceA < distanceB;
		})
	};

	if (maxDistanceIt != objects.end())
	{
		return glm::length((*maxDistanceIt)->Origin() - origin) + (*maxDistanceIt)->GetEarlyOutRadius();
	}

	return 0.0f;
}

glm::vec3 sdf::BVHNode::CalculateBVHExtent(std::vector<sdf::Object*> const& objects, glm::vec3 const& origin)
{
	glm::vec3 extent{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (const auto& obj : objects)
	{
		glm::vec3 distance = glm::abs(obj->Origin() - origin) + glm::vec3(obj->GetEarlyOutRadius());
		extent = glm::max(extent, distance);
	}

	return extent;
}

std::pair<std::vector<sdf::Object*>, std::vector<sdf::Object*>> sdf::BVHNode::SplitObjects(std::vector<sdf::Object*> const& objects)
{
	if (objects.size() <= 1)
	{
		return { objects, {} };
	}

	// Calculate the bounding box of all objects
	glm::vec3 minBounds{ std::numeric_limits<float>::max() };
	glm::vec3 maxBounds{ std::numeric_limits<float>::lowest() };

	for (const auto& obj : objects)
	{
		glm::vec3 origin = obj->Origin();
		minBounds = glm::min(minBounds, origin);
		maxBounds = glm::max(maxBounds, origin);
	}

	// Determine the best split axis and position using SAH
	int bestAxis = 0;
	float bestCost = std::numeric_limits<float>::max();
	size_t bestSplitIndex = 0;

	for (int axis = 0; axis < 3; ++axis)
	{
		std::vector<sdf::Object*> tempObjects{ objects.size() };

		std::transform(objects.begin(), objects.end(), tempObjects.begin(),
			[](sdf::Object* obj)
			{
				return obj;
			});

		std::sort(tempObjects.begin(), tempObjects.end(),
			[axis](const sdf::Object* a, const sdf::Object* b)
			{
				return a->Origin()[axis] < b->Origin()[axis];
			});

		for (size_t i = 1; i < tempObjects.size(); ++i)
		{
			std::vector<sdf::Object*> leftObjects(tempObjects.begin(), tempObjects.begin() + i);
			std::vector<sdf::Object*> rightObjects(tempObjects.begin() + i, tempObjects.end());

			float leftArea = CalculateBoundingBoxArea(leftObjects);
			float rightArea = CalculateBoundingBoxArea(rightObjects);

			float cost = leftArea * leftObjects.size() + rightArea * rightObjects.size();

			if (cost < bestCost)
			{
				bestCost = cost;
				bestAxis = axis;
				bestSplitIndex = i;
			}
		}
	}

	std::vector<sdf::Object*> tempObjects{ objects.size() };

	std::transform(objects.begin(), objects.end(), tempObjects.begin(),
		[](sdf::Object* obj)
		{
			return obj;
		});

	// Sort objects along the best axis and split at the best split index
	std::sort(tempObjects.begin(), tempObjects.end(),
		[bestAxis](const sdf::Object* a, const sdf::Object* b)
		{
			return a->Origin()[bestAxis] < b->Origin()[bestAxis];
		});

	std::vector<sdf::Object*> leftObjects(tempObjects.begin(), tempObjects.begin() + bestSplitIndex);
	std::vector<sdf::Object*> rightObjects(tempObjects.begin() + bestSplitIndex, tempObjects.end());

	return { std::move(leftObjects), std::move(rightObjects) };
}

float sdf::BVHNode::CalculateBoundingBoxArea(std::vector<sdf::Object*> const& objects)
{
	if (objects.empty())
	{
		return 0.0f;
	}

	glm::vec3 minBounds{ std::numeric_limits<float>::max() };
	glm::vec3 maxBounds{ std::numeric_limits<float>::lowest() };

	for (const auto& obj : objects)
	{
		glm::vec3 origin = obj->Origin();
		minBounds = glm::min(minBounds, origin);
		maxBounds = glm::max(maxBounds, origin);
	}

	glm::vec3 extents = maxBounds - minBounds;
	return 2.0f * (extents.x * extents.y + extents.y * extents.z + extents.z * extents.x);
}