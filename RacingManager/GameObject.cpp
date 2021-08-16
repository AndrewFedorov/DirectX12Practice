#include "GameObject.h"
#include "MathHelper.h"
#include "ModelLoader.h"
#include <DirectXMath.h>
#include <string>

using namespace DirectX;

GameObject::GameObject()
{
    objectVariables.world = MathHelper::Identity4x4;
}

void GameObject::Translate(float x, float y, float z)
{
    XMMATRIX translateMatrix = XMMatrixTranslation(x, y, z);
    XMStoreFloat4x4(&objectVariables.world, XMMatrixTranspose(translateMatrix));
}

void GameObject::LoadObject(const std::string& path)
{
    ModelLoader modelLoader(path);
    //modelLoader.LoadModel(*this);
}