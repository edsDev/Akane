#pragma once
#include <numeric>

namespace akane
{
    using akFloat = float;

	constexpr akFloat kFloatEpsilon = 1e-6f;

    constexpr akFloat kFloatZero = 0.f;
    constexpr akFloat kFloatOne  = 1.f;

    constexpr akFloat kPI = 3.141592f;
	constexpr akFloat kInvPI = 1 / kPI;
} // namespace akane