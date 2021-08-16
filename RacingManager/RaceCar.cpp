#include "RaceCar.h"
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

RaceCar::RaceCar(const std::vector<XMFLOAT3>& trajectory, unsigned int speed)
{
    this->trajectory = trajectory;
    this->speed = speed;
}

void RaceCar::Run()
{
    const XMFLOAT3& pos = trajectory[position];
    position = (position + speed) % trajectory.size();
    Translate(pos.x, pos.y, pos.z);
}