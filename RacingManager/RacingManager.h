#pragma once

#include "DirectXApp.h"
#include "MathHelper.h"
#include "FrameResource.h"
#include "Mesh.h"
#include "GameObject.h"
#include "RaceCar.h"
#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

constexpr int NUN_FRAME_RESOURCE = 3;

class RacingManager : public DirectXApp
{
public:
    RacingManager(HINSTANCE hInstance);
    RacingManager(const RacingManager&) = delete;
    RacingManager& operator=(const RacingManager&) = delete;
    ~RacingManager();

    virtual void Initialize() override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt) override;
    virtual void Draw(const GameTimer& gt ) override;

    LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

    void UpdateCamera(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);

    void BuildRootSignature();
    void BuildShadersAndInputLayout();

    void LoadGameObjects();
    void BuildPSOs();
    void BuildFrameResources();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const GameObjectMap& gameObjects);

    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint, const std::string& target);

    void GameMechanic(const GameTimer& gt);

    std::vector<std::unique_ptr<FrameResource>> frameResources;
    FrameResource* currentFrameResource = nullptr;
    int currentFrameResourceIndex = 0;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> shaders;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> PSOs;
    GameObjectMap gameObjects;

    DirectX::XMFLOAT3 eyePos = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4;
    DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4;

    float theta = 1.5f * DirectX::XM_PI;
    float phi = DirectX::XM_PIDIV2 - 0.1f;
    float radius = 5.0f;

    float lastTick = 0.0f;

    POINT lastMousePos = { 0, 0 };
};