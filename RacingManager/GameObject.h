#pragma once

#include "Mesh.h"
#include <DirectXMath.h>
#include <string>
#include <memory>
#include <unordered_map>

struct ObjectVariables
{
    DirectX::XMFLOAT4X4 world;
};

class GameObject
{
public:
    GameObject();

    void Translate(float x, float y, float z);
    void LoadObject(const std::string& path);

    virtual void dummy() {}

    Mesh mesh;
    ObjectVariables objectVariables;
};

using GameObjectMap = std::unordered_map<std::string, std::unique_ptr<GameObject>>;
using GameObjectPair = std::pair<std::string, std::unique_ptr<GameObject>>;