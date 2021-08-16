#include "RacingManager.h"
#include "DirectXApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "FrameResource.h"
#include "GameObject.h"
#include "Microsoft/d3dx12.h"
#include "RaceCar.h"
#include "ModelLoader.h"
#include <WindowsX.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <D3Dcompiler.h>
#include <memory>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

RacingManager::RacingManager(HINSTANCE hInstance): DirectXApp(hInstance) {}

RacingManager::~RacingManager()
{
    if(device != nullptr)
        FlushCommandQueue();
}

LRESULT RacingManager::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    }

    return DirectXApp::MsgProc(hwnd, msg, wParam, lParam);
}

void RacingManager::Initialize()
{
    DirectXApp::Initialize();

    commandList->Reset(commandAllocator.Get(), nullptr);

    BuildRootSignature();
    BuildShadersAndInputLayout();
    LoadGameObjects();
    BuildFrameResources();
    BuildPSOs();

    commandList->Close();
    ID3D12CommandList* cmdsLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();
}
 
void RacingManager::BuildRootSignature()
{
    CD3DX12_ROOT_PARAMETER slotRootParameter[2] = { CD3DX12_ROOT_PARAMETER(), CD3DX12_ROOT_PARAMETER() };
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());

    device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(rootSignature.GetAddressOf()));
}

void RacingManager::BuildShadersAndInputLayout()
{
    shaders["standardVS"] = CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_0");
    shaders["opaquePS"] = CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_0");

    inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

ComPtr<ID3DBlob> RacingManager::CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines,
    const std::string& entrypoint, const std::string& target)
{
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = S_OK;
    ComPtr<ID3DBlob> byteCode = nullptr;
    ComPtr<ID3DBlob> errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if(errors != nullptr)
        OutputDebugStringA((char*)errors->GetBufferPointer());

    return byteCode;
}

void RacingManager::LoadGameObjects()
{
    /*
    auto trajectory = std::make_unique<GameObject>();
    trajectory->LoadObject("Models/redbullring2.dae");
    trajectory->mesh.CreateBuffers(device, commandList);
    gameObjects["Trajectory"] = std::move(trajectory);

    ModelLoader modelLoader;
    std::vector<XMFLOAT3> points = modelLoader.LoadFloat3Vector("Models/redbullring2");

    auto raceCar1 = std::make_unique<RaceCar>(points, 2);
    raceCar1->LoadObject("Models/Car.dae");
    raceCar1->mesh.CreateBuffers(device, commandList);
    gameObjects["RaceCar1"] = std::move(raceCar1);

    auto raceCar2 = std::make_unique<RaceCar>(points, 1);
    raceCar2->LoadObject("Models/Car.dae");
    raceCar2->mesh.CreateBuffers(device, commandList);
    gameObjects["RaceCar2"] = std::move(raceCar2);

    */

    ModelLoader modelLoader("Models/RaceTrack.dae");
    modelLoader.LoadModel(gameObjects);
    for(const auto& gameObjectPair : gameObjects)
        gameObjectPair.second->mesh.CreateBuffers(device, commandList);
}

void RacingManager::BuildFrameResources()
{
    for(int i = 0; i < NUN_FRAME_RESOURCE; ++i)
        frameResources.push_back(std::make_unique<FrameResource>(device.Get(), 1, static_cast<UINT>(gameObjects.size())));
}

void RacingManager::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
    opaquePsoDesc.pRootSignature = rootSignature.Get();
    opaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["standardVS"]->GetBufferPointer()),
        shaders["standardVS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["opaquePS"]->GetBufferPointer()),
        shaders["opaquePS"]->GetBufferSize()
    };

    CD3DX12_RASTERIZER_DESC rd = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    rd.FillMode = D3D12_FILL_MODE_SOLID;
    rd.CullMode = D3D12_CULL_MODE_NONE;
    rd.FrontCounterClockwise = TRUE;

    opaquePsoDesc.RasterizerState = rd;
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    opaquePsoDesc.NumRenderTargets = 1;
    opaquePsoDesc.RTVFormats[0] = backBufferFormat;
    opaquePsoDesc.SampleDesc.Count = 1;
    opaquePsoDesc.SampleDesc.Quality = 0;
    opaquePsoDesc.DSVFormat = depthStencilFormat;
    device->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&PSOs["opaque"]));
}

void RacingManager::OnResize()
{
    DirectXApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void RacingManager::GameMechanic(const GameTimer& gt)
{
    float totalTime = gt.TotalTime();
    if(totalTime > lastTick)
    {
        /*
        RaceCar* raceCar1 = dynamic_cast<RaceCar*>(gameObjects["RaceCar1"].get());
        raceCar1->Run();
        RaceCar* raceCar2 = dynamic_cast<RaceCar*>(gameObjects["RaceCar2"].get());
        raceCar2->Run();
        lastTick = totalTime + 0.1f;
        */
    }

}

void RacingManager::Update(const GameTimer& gt)
{
    UpdateCamera(gt);
    GameMechanic(gt);

    currentFrameResourceIndex = (currentFrameResourceIndex + 1) % NUN_FRAME_RESOURCE;
    currentFrameResource = frameResources[currentFrameResourceIndex].get();

    if(currentFrameResource->fence != 0 && fence->GetCompletedValue() < currentFrameResource->fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        fence->SetEventOnCompletion(currentFrameResource->fence, eventHandle);
        if(eventHandle)
        {
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    UpdateObjectCBs(gt);
    UpdateMainPassCB(gt);
}

void RacingManager::UpdateCamera(const GameTimer& gt)
{
    eyePos.x = radius * sinf(phi) * cosf(theta);
    eyePos.z = radius * sinf(phi) * sinf(theta);
    eyePos.y = radius * cosf(phi);

    XMVECTOR pos = XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);
}

void RacingManager::UpdateObjectCBs(const GameTimer& gt)
{
    size_t i = 0;
    auto currentObjectCB = currentFrameResource->objectCB.get();
    for(const auto& gameObjectPair : gameObjects)
    {
        GameObject* gameObject = gameObjectPair.second.get();
        //if(gameObject.numFramesChanged > 0)
        {
            currentObjectCB->CopyData<ObjectVariables>(i++, gameObject->objectVariables);
        //    gameObject.numFramesChanged -= 1;
        }
    }
}

void RacingManager::UpdateMainPassCB(const GameTimer& gt)
{
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    PassConstants mMainPassCB;
    XMStoreFloat4x4(&mMainPassCB.viewProj, XMMatrixTranspose(viewProj));

    auto currPassCB = currentFrameResource->passCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void RacingManager::Draw(const GameTimer& gt)
{
    auto cmdListAlloc = currentFrameResource->ñommandAllocator;

    cmdListAlloc->Reset();

    commandList->Reset(cmdListAlloc.Get(), PSOs["opaque"].Get());

    commandList->RSSetViewports(1, &screenViewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &resourceBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();
    commandList->ClearRenderTargetView(currentBackBufferView, Colors::SeaGreen, 0, nullptr);
    commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    commandList->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);

    commandList->SetGraphicsRootSignature(rootSignature.Get());

    auto passCB = currentFrameResource->passCB->Resource();
    commandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

    DrawRenderItems(commandList.Get(), gameObjects);

    resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &resourceBarrier);

    commandList->Close();

    ID3D12CommandList* cmdsLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    swapChain->Present(0, 0);
    currentBackBuffer = (currentBackBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

    currentFrameResource->fence = ++currentFence;

    commandQueue->Signal(fence.Get(), currentFence);
}

void RacingManager::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const GameObjectMap& gameObjects)
{
    UINT objCBByteSize = UploadBuffer::CalcConstantBufferByteSize(sizeof(ObjectVariables));
    auto objectCB = currentFrameResource->objectCB->Resource();

    size_t i = 0;
    for(const auto& gameObjectPair : gameObjects)
    {
        GameObject gameObject = *gameObjectPair.second.get();

        D3D12_VERTEX_BUFFER_VIEW vertexBufferView = gameObject.mesh.GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW indexBufferView = gameObject.mesh.GetIndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
        cmdList->IASetIndexBuffer(&indexBufferView);
        cmdList->IASetPrimitiveTopology(gameObject.mesh.primitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + static_cast<UINT64>(i++)*objCBByteSize;
        cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
        for(const std::pair<std::string, SubMesh>& submeshGeometryPair : gameObject.mesh.subMeshes)
        {
            const SubMesh& submesh = submeshGeometryPair.second;
            cmdList->DrawIndexedInstanced(submesh.indexCount, 1, submesh.startIndexLocation, submesh.baseVertexLocation, 0);
            //cmdList->DrawInstanced(submesh.vertexCount, 1, submesh.startVertexLocation, 0);
        }
    }
}

void RacingManager::OnMouseDown(WPARAM btnState, int x, int y)
{
    lastMousePos.x = x;
    lastMousePos.y = y;

    SetCapture(hMainWnd);
}

void RacingManager::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void RacingManager::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - lastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - lastMousePos.y));

        theta += dx;
        phi += dy;

        phi = MathHelper::Clamp(phi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        float dx = 0.2f*static_cast<float>(x - lastMousePos.x);
        float dy = 0.2f*static_cast<float>(y - lastMousePos.y);

        radius += dx - dy;

        radius = MathHelper::Clamp(radius, 5.0f, 150.0f);
    }

    lastMousePos.x = x;
    lastMousePos.y = y;
}