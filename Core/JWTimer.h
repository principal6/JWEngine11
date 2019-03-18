#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWTimer
	{
	public:
		JWTimer();

		auto GetElapsedTimeMilliSec()->long long;

		void ResetTimer();

	private:
		std::chrono::time_point<std::chrono::steady_clock> m_PreviousTimePoint{};
	};
};