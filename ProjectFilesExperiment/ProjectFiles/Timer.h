#pragma once

//Standard includes
#include <chrono>
#include <cstdint>
#include <vector>
#include <iostream>

#include "Misc.h"

namespace sdf
{
	class GameTimer final
	{
	public:
		GameTimer();
		virtual ~GameTimer() = default;

		GameTimer(const GameTimer&) = delete;
		GameTimer(GameTimer&&) noexcept = delete;
		GameTimer& operator=(const GameTimer&) = delete;
		GameTimer& operator=(GameTimer&&) noexcept = delete;

		void StartBenchmark(std::string const& sceneName, sdf::ResultStats const& hitStats, sdf::ResultStats const& missStats);
		void Update();
		
		float GetElapsed() const { return m_ElapsedTime; }

		float& SetBenchmarkTargetFrames() { return m_BenchmarkTargetTime; }
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_PreviousTime;

		float m_OutputTimer{ 0.0f };
		uint32_t m_OutputFPSCount{ 0 };
		
		float m_TotalTime{ 0.0f };
		uint32_t m_TotalFPSCount{ 0 };

		float m_ElapsedTime{ 0.0f };

		bool m_BenchmarkActive{ false };
		float m_BenchmarkTime{ 0.0f };
		float m_BenchmarkTargetTime{ 11.f };
		std::vector<float> m_BenchmarkFrameTimeVec{};

		std::string m_CurrentSceneName{};
		ResultStats m_HitStats{};
		ResultStats m_MissStats{};

		void PrintFPS() const;
		void EndBenchmark();
		void OutputBenchmarkResults(std::vector<float> const& frameTimeVec, std::ostream& outputStream);
	};
}
