#pragma once
#include "ColorRGB.h"

namespace sdf
{

	struct HitRecord
	{
		bool DidHit{ false };

		float Distance{};

		int TotalSteps{};
		int EarlyOutUsage{};

		int BVHDepth{};

		ColorRGB Shade{ 0.f, 0.f, 0.f };
	};

	struct ResultStats
	{
		int Count{};

		int AverageStepsThroughScene{};
		int AverageEarlyOutSteps{};

		int AverageBVHDepth{};
	};

}