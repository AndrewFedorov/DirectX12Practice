#pragma once

#include <d3d12.h>
#include <wrl.h>

class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device3* device, UINT elementByteSize, UINT elementCount, bool isConstantBuffer);
    UploadBuffer(const UploadBuffer&) = delete;
    UploadBuffer& operator=(const UploadBuffer&) = delete;
    ~UploadBuffer();

    static UINT CalcConstantBufferByteSize(UINT elementByteSize);
    ID3D12Resource* Resource() const;

    template<typename T>
    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&mappedData[elementIndex * elementByteSize], &data, sizeof(T));
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    BYTE* mappedData = nullptr;
    UINT elementByteSize = 0;
};