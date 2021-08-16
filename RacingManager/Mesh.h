#pragma once

#include <windows.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <string>
#include <unordered_map>
#include <vector>

struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 normal;
};

struct SubMesh
{
    UINT indexCount = 0;
    UINT startIndexLocation = 0;
    INT baseVertexLocation = 0;

    UINT vertexCount = 0;
    UINT startVertexLocation = 0;
};

class Mesh
{
public:
    Mesh() = default;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    D3D12_PRIMITIVE_TOPOLOGY primitiveType;

    UINT vertexByteStride = 0;
    UINT vertexBufferByteSize = 0;
    UINT indexBufferByteSize = 0;

    std::unordered_map<std::string, SubMesh> subMeshes;

    void CreateBuffers(Microsoft::WRL::ComPtr<ID3D12Device3> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;
};