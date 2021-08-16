#pragma once

#include <DirectXMath.h>

namespace MathHelper
{
    constexpr float Pi = 3.1415926535f;

    constexpr DirectX::XMFLOAT4X4 Identity4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    template<typename T>
    T Clamp(const T& x, const T& low, const T& high)
    {
        return x < low ? low : (x > high ? high : x); 
    }
}