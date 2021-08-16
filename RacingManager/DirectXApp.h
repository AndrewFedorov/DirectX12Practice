#pragma once

#include "GameTimer.h"
#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <string>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DirectXApp
{
public:
    static DirectXApp* GetInstance();

    int Run();
    virtual void Initialize();
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    DirectXApp(HINSTANCE hInstance);
    DirectXApp(const DirectXApp&) = delete;
    DirectXApp& operator=(const DirectXApp&) = delete;
    virtual ~DirectXApp();

    virtual void OnResize();
    virtual void Update(const GameTimer&) = 0;
    virtual void Draw(const GameTimer&) = 0;

    void InitMainWindow();
    void InitDirect3D();
    void CreateCommandObjects();
    void CreateSwapChain();
    void CreateRtvDescriptorHeap();
    void CreateDsvDescriptorHeap();
    void FlushCommandQueue();

    float GetAspectRatio() const;
    ID3D12Resource* CurrentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

    static DirectXApp* directXApp;

    HINSTANCE hInstance = nullptr;
    HWND      hMainWnd = nullptr;

    GameTimer gameTimer;

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device3> device;

    UINT64 currentFence = 0;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

    static constexpr int SWAP_CHAIN_BUFFER_COUNT = 2;
    int currentBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer[SWAP_CHAIN_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;

    D3D12_VIEWPORT screenViewport;
    D3D12_RECT scissorRect;

    UINT rtvDescriptorSize = 0;
    UINT dsvDescriptorSize = 0;
    UINT cbvSrvUavDescriptorSize = 0;

    std::wstring appName = L"DirectX app";
    D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    UINT clientWidth = 1920;
    UINT clientHeight = 1080;
    UINT numerator = 60;
    UINT denominator = 1;
};