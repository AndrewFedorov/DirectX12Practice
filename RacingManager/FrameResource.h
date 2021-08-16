#pragma once

#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GameObject.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>

struct PassConstants
{
    DirectX::XMFLOAT4X4 viewProj = MathHelper::Identity4x4;
};

struct FrameResource
{
public:
    
    FrameResource(ID3D12Device3* device, UINT passCount, UINT objectCount);
    ~FrameResource();

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> ñommandAllocator;

    std::unique_ptr<UploadBuffer> passCB = nullptr;
    std::unique_ptr<UploadBuffer> objectCB = nullptr;

    UINT64 fence = 0;
};