#include "FrameResource.h"
#include "UploadBuffer.h"
#include "GameObject.h"
#include <d3d12.h>
#include <memory>

FrameResource::FrameResource(ID3D12Device3* device, UINT passCount, UINT objectCount)
{
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(ñommandAllocator.GetAddressOf()));

    passCB = std::make_unique<UploadBuffer>(device, sizeof(PassConstants), passCount, true);
    objectCB = std::make_unique<UploadBuffer>(device, sizeof(ObjectVariables), objectCount, true);
}

FrameResource::~FrameResource()
{

}