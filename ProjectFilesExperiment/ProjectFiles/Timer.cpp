#include "Timer.h"

#include <iostream>
#include <numeric>
#include <fstream>
#include <variant>

#include "Renderer.h"
#include "SDL.h"
#include "BVHNode.h"
#include "SDFObjects.h"

sdf::GameTimer::GameTimer()
{
	m_StartTime = std::chrono::high_resolution_clock::now();
	m_PreviousTime = m_StartTime;
}

void sdf::GameTimer::StartBenchmark(std::string const& sceneName, sdf::ResultStats const& hitStats, sdf::ResultStats const& missStats)
{
	if (m_BenchmarkActive)
	{
		std::cout << "Benchmark already active\n";
		return;
	}

	m_BenchmarkActive = true;
	m_BenchmarkTime = 0.0f;
	m_BenchmarkFrameTimeVec.clear();
	m_BenchmarkFrameTimeVec.reserve(60 * m_BenchmarkTargetTime);

	m_CurrentSceneName = sceneName;
	m_HitStats = hitStats;
	m_MissStats = missStats;

	std::cout << "**BENCHMARK STARTED**\n";
}

void sdf::GameTimer::Update()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = currentTime - m_PreviousTime;
	m_ElapsedTime = elapsed.count();
	m_PreviousTime = currentTime;

	m_TotalTime += m_ElapsedTime;
	++m_TotalFPSCount;

	//reset after 1 second -> so fpS
	//if (m_OutputTimer >= 1.0f)
	//{
	//	m_TotalTime += m_OutputTimer;
	//	m_TotalFPSCount += m_OutputFPSCount;
	//	
	//	m_OutputFPSCount = 0;
	//	m_OutputTimer = 0.0f;
	//}

	if (m_BenchmarkActive)
	{
		m_BenchmarkTime += m_ElapsedTime;
		m_BenchmarkFrameTimeVec.emplace_back(m_ElapsedTime); 

		if (m_BenchmarkTime >= m_BenchmarkTargetTime)
		{
			EndBenchmark();
		}
	}
}

void sdf::GameTimer::PrintFPS() const
{
	std::cout << "FPS: " << m_OutputFPSCount << "\n";
	std::cout << "AVG FPS: " << m_TotalFPSCount / m_TotalTime << " <= (" << m_TotalFPSCount << " : " << m_TotalTime << ")\n";
}

void sdf::GameTimer::EndBenchmark()
{
	m_BenchmarkActive = false;

	OutputBenchmarkResults(m_BenchmarkFrameTimeVec, std::cout);
	
	std::ofstream fileStream("benchmark.csv", std::ios::app);
	OutputBenchmarkResults(m_BenchmarkFrameTimeVec, fileStream);
	fileStream.close();
}

void sdf::GameTimer::OutputBenchmarkResults(std::vector<float> const& frameTimeVec, std::ostream& outputStream)
{
	if (frameTimeVec.empty()) return;

	std::vector<float> sortedFrameTimes{ frameTimeVec };
	std::sort(sortedFrameTimes.begin(), sortedFrameTimes.end());

	size_t const numElementsToRemove = static_cast<size_t>(sortedFrameTimes.size() * 0.05);

	sortedFrameTimes.erase(sortedFrameTimes.begin(), sortedFrameTimes.begin() + numElementsToRemove);
	sortedFrameTimes.erase(sortedFrameTimes.end() - numElementsToRemove, sortedFrameTimes.end());

	double const benchMarkTotalTime{ std::accumulate(sortedFrameTimes.begin(), sortedFrameTimes.end(), 0.0f) };
	double const avgFrameTime{ benchMarkTotalTime / sortedFrameTimes.size() };

	std::ifstream fileCheck("benchmark.csv");
	bool isEmpty{ fileCheck.peek() == std::ifstream::traits_type::eof() };
	fileCheck.close();

	char constexpr delimiter{ ',' };

	if (isEmpty)
	{
		outputStream << "\n";
		outputStream
			<< delimiter << "SCENE COMPLEXITY"
			<< delimiter << "EARLY OUT"
			<< delimiter << "BOX EARLY OUT"
			<< delimiter << "BVH"
			<< delimiter << "BOX BVH"
			<< delimiter << "TOTAL TIME"
			<< delimiter << "TOTAL FRAMES"
			<< delimiter << "AVG TIME"
			<< delimiter << "AVG FPS"
			<< delimiter << "HIT RAYS"
			<< delimiter << "AVG STEPS"
			<< delimiter << "AVG EARLY OUT"
			<< delimiter << "AVG BVH DEPTH"
			<< delimiter << "MISSED RAYS"
			<< delimiter << "AVG STEPS"
			<< delimiter << "AVG EARLY OUT"
			<< delimiter << "AVG BVH DEPTH"
			<< delimiter << "FRAME TIMES\n";
	}

	outputStream << delimiter
		<< m_CurrentSceneName << delimiter
		<< std::boolalpha << Scene::m_UseEarlyOut << delimiter
		<< std::boolalpha << Object::m_UseBoxEarlyOut << delimiter
		<< std::boolalpha << Scene::m_UseBVH << delimiter
		<< std::boolalpha << BVHNode::m_BoxBVH << delimiter
		<< std::to_string(benchMarkTotalTime) << delimiter
		<< std::to_string(sortedFrameTimes.size()) << delimiter
		<< std::to_string(avgFrameTime) << delimiter
		<< std::to_string(1 / avgFrameTime) << delimiter
		<< std::to_string(m_HitStats.Count) << delimiter
		<< std::to_string(m_HitStats.AverageStepsThroughScene) << delimiter
		<< std::to_string(m_HitStats.AverageEarlyOutSteps) << delimiter
		<< std::to_string(m_HitStats.AverageBVHDepth) << delimiter
		<< std::to_string(m_MissStats.Count) << delimiter
		<< std::to_string(m_MissStats.AverageStepsThroughScene) << delimiter
		<< std::to_string(m_MissStats.AverageEarlyOutSteps) << delimiter
		<< std::to_string(m_MissStats.AverageBVHDepth) << delimiter;

	for (const auto& time : sortedFrameTimes)
	{
		outputStream << time << delimiter;
	}
	outputStream << "\n";
}