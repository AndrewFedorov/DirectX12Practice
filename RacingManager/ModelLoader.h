#pragma once
#include "GameObject.h"
#include "RapidXML/rapidxml.hpp"
#include <DirectXMath.h>
#include <string>
#include <vector>

class ModelLoader
{
public:
    ModelLoader(const std::string& path);
    ~ModelLoader();

    void LoadModel(GameObjectMap& gameObjects);

    std::vector<DirectX::XMFLOAT3> LoadFloat3Vector(const std::string& path);

private:
    rapidxml::xml_document<> xmlDocument;
};