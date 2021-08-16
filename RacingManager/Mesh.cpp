#include "Mesh.h"
#include "Microsoft/d3dx12.h"
#include <wrl.h>
#include <d3d12.h>
#include <string>

#pragma comment(lib,"dxguid.lib")

using Microsoft::WRL::ComPtr;

D3D12_VERTEX_BUFFER_VIEW Mesh::GetVertexBufferView() const
{
    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
    vbv.StrideInBytes = vertexByteStride;
    vbv.SizeInBytes = vertexBufferByteSize;
    return vbv;
}

D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexBufferView() const
{
    D3D12_INDEX_BUFFER_VIEW ibv;
    ibv.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
    ibv.Format = DXGI_FORMAT_R32_UINT;
    ibv.SizeInBytes = indexBufferByteSize;
    return ibv;
}

ComPtr<ID3D12Resource> CreateDefaultBuffer(ComPtr<ID3D12Device3> device, ComPtr<ID3D12GraphicsCommandList> commandList,
    const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer)
{
    ComPtr<ID3D12Resource> defaultBuffer;

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(defaultBuffer.GetAddressOf()));

    heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(uploadBuffer.GetAddressOf()));

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    commandList->ResourceBarrier(1, &resourceBarrier);
    UpdateSubresources<1>(commandList.Get(), defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

    resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    commandList->ResourceBarrier(1, &resourceBarrier);

    return defaultBuffer;
}

void Mesh::CreateBuffers(ComPtr<ID3D12Device3> device, ComPtr<ID3D12GraphicsCommandList> commandList)
{
    vertexByteStride = sizeof(Vertex);
    vertexBufferByteSize = static_cast<UINT>(vertices.size()) * vertexByteStride;
    indexBufferByteSize = static_cast<UINT>(indices.size()) * sizeof(std::uint32_t);
    primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    vertexBufferGPU = CreateDefaultBuffer(device.Get(), commandList.Get(), vertices.data(),
        vertexBufferByteSize, vertexBufferUploader);
    indexBufferGPU = CreateDefaultBuffer(device.Get(), commandList.Get(), indices.data(),
        indexBufferByteSize, indexBufferUploader);

    SubMesh& subMesh = subMeshes["main"];
    subMesh.indexCount = static_cast<UINT>(indices.size());
    subMesh.vertexCount = static_cast<UINT>(vertices.size());
    subMesh.startIndexLocation = 0;
    subMesh.startVertexLocation = 0;
    subMesh.baseVertexLocation = 0;
}