#include "DirectXApp.h"
#include "AppException.h"
#include "Microsoft/d3dx12.h"
#include <Windows.h>
#include <wrl.h>
#include <dxgi.h>
#include <d3d12.h>
#include <string>

using Microsoft::WRL::ComPtr;

DirectXApp* DirectXApp::directXApp = nullptr;
DirectXApp* DirectXApp::GetInstance()
{
    return directXApp;
}

DirectXApp::DirectXApp(HINSTANCE hInstance): hInstance(hInstance)
{
    scissorRect = {};
    screenViewport = {};
    directXApp = this;
}

DirectXApp::~DirectXApp()
{
    if(device != nullptr)
        FlushCommandQueue();
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DirectXApp::GetInstance()->MsgProc(hwnd, msg, wParam, lParam);
}

LRESULT DirectXApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);
    case WM_KEYUP:
        if(wParam == VK_ESCAPE)
            PostQuitMessage(0);

        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void DirectXApp::Initialize()
{
    InitMainWindow();
    InitDirect3D();
    OnResize();
}

void DirectXApp::InitMainWindow()
{
    std::wstring className = L"MainWnd";
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = className.c_str();

    if(!RegisterClass(&wc))
        throw AppException("Register class error");

    DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
    AdjustWindowRect(&rect, windowStyle, false);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    hMainWnd = CreateWindow(className.c_str(), appName.c_str(), windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
        width, height, 0, 0, hInstance, 0);

    if(!hMainWnd)
        throw AppException("Create window error");

    ShowWindow(hMainWnd, SW_SHOW);
    UpdateWindow(hMainWnd);
}

void DirectXApp::InitDirect3D()
{
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    cbvSrvUavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvDescriptorHeap();
    CreateDsvDescriptorHeap();
}

void DirectXApp::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf()));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(),
        nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));
    commandList->Close();
}

void DirectXApp::CreateSwapChain()
{
    swapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = clientWidth;
    sd.BufferDesc.Height = clientHeight;
    sd.BufferDesc.RefreshRate.Numerator = numerator;
    sd.BufferDesc.RefreshRate.Denominator = denominator;
    sd.BufferDesc.Format = backBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
    sd.OutputWindow = hMainWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    factory->CreateSwapChain(commandQueue.Get(), &sd, swapChain.GetAddressOf());
}

void DirectXApp::CreateRtvDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf()));
}

void DirectXApp::CreateDsvDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf()));
}

void DirectXApp::OnResize()
{
    FlushCommandQueue();

    commandList->Reset(commandAllocator.Get(), nullptr);

    for(int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
        swapChainBuffer[i].Reset();
    depthStencilBuffer.Reset();

    swapChain->ResizeBuffers(
        SWAP_CHAIN_BUFFER_COUNT,
        clientWidth, clientHeight,
        backBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    currentBackBuffer = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for(UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
    {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffer[i]));
        device->CreateRenderTargetView(swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, rtvDescriptorSize);
    }

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = clientWidth;
    depthStencilDesc.Height = clientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = depthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
    device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON,
        &optClear, IID_PPV_ARGS(depthStencilBuffer.GetAddressOf()));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = depthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

    CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    commandList->ResourceBarrier(1, &resourceBarrier);

    commandList->Close();
    ID3D12CommandList* cmdsLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    screenViewport.TopLeftX = 0;
    screenViewport.TopLeftY = 0;
    screenViewport.Width = static_cast<float>(clientWidth);
    screenViewport.Height = static_cast<float>(clientHeight);
    screenViewport.MinDepth = 0.0f;
    screenViewport.MaxDepth = 1.0f;

    scissorRect = { 0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
}

void DirectXApp::FlushCommandQueue()
{
    currentFence++;
    commandQueue->Signal(fence.Get(), currentFence);

    if(fence->GetCompletedValue() < currentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        fence->SetEventOnCompletion(currentFence, eventHandle);
        if(eventHandle)
        {
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }
}

int DirectXApp::Run()
{
    MSG msg = { 0 };

    gameTimer.Reset();
    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            gameTimer.Tick();
            Update(gameTimer);
            Draw(gameTimer);
        }
    }

    return static_cast<int>(msg.wParam);
}

float DirectXApp::GetAspectRatio() const
{
    return static_cast<float>(clientWidth) / clientHeight;
}

ID3D12Resource* DirectXApp::CurrentBackBuffer() const
{
    return swapChainBuffer[currentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXApp::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentBackBuffer, rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXApp::DepthStencilView() const
{
    return dsvHeap->GetCPUDescriptorHandleForHeapStart();
}