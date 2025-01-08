#pragma once
#include "glm/glm.hpp"

#include <memory>
#include <string>
#include <vector>

namespace sdf
{
	struct HitRecord;
	struct Camera;

	class Object;
	class BVHNode;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		//returns the distance and the number of steps
		HitRecord GetClosestHit(const glm::vec3& origin, const glm::vec3& direction, float minDistance, float maxDistance, int maxSteps) const;

		void Update(float ElapsedSec);

		Camera const& GetCamera() const { return m_Camera; }

		void CreateBVHStructure();

		static bool m_UseEarlyOut;
		static bool m_UseBVH;

		//static int m_BVHSteps;
		static void MoveCameraPos(float moveDistance);
	protected:
		//vector needs full definition upon construction, so it can call the destructor of the unique_ptrs
		std::vector<std::unique_ptr<Object>> m_SDObjectUPtrVec;
		static Camera m_Camera;
		static bool m_CameraMoved;

	private:
		std::unique_ptr<BVHNode> m_BVHRoot{ nullptr };

		std::pair<float, const sdf::Object*> GetDistanceToScene(const glm::vec3& point, HitRecord& outHitRecord) const;

	};

}
