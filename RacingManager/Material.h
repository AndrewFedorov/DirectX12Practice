#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>

struct MaterialProperties
{
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.25f;

    //DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4;
};


class Material
{
public:

    std::string Name;

    int MatCBIndex = -1;
    int DiffuseSrvHeapIndex = -1;
    int NormalSrvHeapIndex = -1;

    MaterialProperties properties;

    //DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct Texture
{
    // Unique material name for lookup.
    std::string Name;

    std::wstring Filename;

    Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};