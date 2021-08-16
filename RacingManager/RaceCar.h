#pragma once

#include "GameObject.h"
#include <DirectXMath.h>
#include <vector>

class RaceCar : public GameObject
{
public:
    RaceCar(const std::vector<DirectX::XMFLOAT3>& trajectory, unsigned int speed);

    void Run();

    void dummy() override {};

    std::vector<DirectX::XMFLOAT3> trajectory;
    unsigned int position = 0;
    unsigned int speed = 1;
};