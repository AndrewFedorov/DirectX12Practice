#include "UploadBuffer.h"
#include "Microsoft/d3dx12.h"
#include <d3d12.h>

UploadBuffer::UploadBuffer(ID3D12Device3* device, UINT elementByteSizeArg, UINT elementCount, bool isConstantBuffer)
{
    elementByteSize = elementByteSizeArg;
    if(isConstantBuffer)
        elementByteSize = CalcConstantBufferByteSize(elementByteSize);

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(elementByteSize)*elementCount);
    device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

    uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
}

UploadBuffer::~UploadBuffer()
{
    if(uploadBuffer != nullptr)
        uploadBuffer->Unmap(0, nullptr);

    mappedData = nullptr;
}

ID3D12Resource* UploadBuffer::Resource() const
{
    return uploadBuffer.Get();
}

UINT UploadBuffer::CalcConstantBufferByteSize(UINT elementByteSize)
{
    return (elementByteSize + 255) & ~255;
}