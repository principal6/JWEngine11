#include "JWTimer.h"

using namespace JWEngine;

JWTimer::JWTimer()
{
	ResetTimer();
}

auto JWTimer::GetElapsedTimeMilliSec()->long long
{
	auto result = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_PreviousTimePoint);

	return result.count();
}

void JWTimer::ResetTimer()
{
	m_PreviousTimePoint = std::chrono::high_resolution_clock::now();
}