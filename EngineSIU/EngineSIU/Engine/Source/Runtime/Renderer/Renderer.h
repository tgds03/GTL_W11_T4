#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include <d3dcompiler.h>

#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Set.h"

#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDBufferManager.h"


class FCascadeParticleRenderPass;
class FLightHeatMapRenderPass;
class FPostProcessCompositingPass;
enum class EResourceType : uint8;

class FSceneRenderPass;
class UWorld;
class UObject;

class FDXDShaderManager;
class FEditorViewportClient;

class FViewportResource;

class FStaticMeshRenderPass;
class FSkeletalMeshRenderPass;
class FWorldBillboardRenderPass;
class FEditorBillboardRenderPass;
class FGizmoRenderPass;
class FUpdateLightBufferPass;
class FDepthBufferDebugPass;
class FWorldNormalDebugPass;
class FLineRenderPass;
class FFogRenderPass;
class FCameraEffectRenderPass;
class FCompositingPass;
class FSlateRenderPass;
class FEditorRenderPass;
class FDepthPrePass;
class FTileLightCullingPass;
class FGPUTimingManager;

class FRenderer
{
public:
    //==========================================================================
    // 초기화/해제 관련 함수
    //==========================================================================
    void Initialize(FGraphicsDevice* InGraphics, FDXDBufferManager* InBufferManager, FGPUTimingManager* InGPUTimingManager);
    void Release();

    //==========================================================================
    // 렌더 패스 관련 함수
    //==========================================================================
    void Render(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void RenderViewport(const std::shared_ptr<FEditorViewportClient>& Viewport) const; // TODO: 추후 RenderSlate로 변경해야함

protected:
    void BeginRender(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void UpdateCommonBuffer(const std::shared_ptr<FEditorViewportClient>& Viewport) const;
    void PrepareRender(FViewportResource* ViewportResource) const;
    void PrepareRenderPass() const;
    void RenderWorldScene(const std::shared_ptr<FEditorViewportClient>& Viewport) const;
    //void RenderTranslucency(const std::shared_ptr<FEditorViewportClient>& Viewport) const;
    void RenderPostProcess(const std::shared_ptr<FEditorViewportClient>& Viewport) const;
    void RenderEditorOverlay(const std::shared_ptr<FEditorViewportClient>& Viewport) const;

    void EndRender();
    void ClearRenderArr() const;
    
    //==========================================================================
    // 버퍼 생성/해제 함수 (템플릿 포함)
    //==========================================================================
public:
    template<typename T>
    ID3D11Buffer* CreateImmutableVertexBuffer(const FString& key, const TArray<T>& Vertices);

    ID3D11Buffer* CreateImmutableIndexBuffer(const FString& key, const TArray<uint32>& indices);
    
    // 상수 버퍼 생성/해제
    void CreateConstantBuffers();
    void ReleaseConstantBuffer() const;

    void CreateCommonShader() const;

public:
    FGraphicsDevice* Graphics;
    FDXDBufferManager* BufferManager;
    FDXDShaderManager* ShaderManager = nullptr;
    class FShadowManager* ShadowManager = nullptr;
    FGPUTimingManager* GPUTimingManager = nullptr;
    
    class FShadowRenderPass* ShadowRenderPass;

    FStaticMeshRenderPass* StaticMeshRenderPass = nullptr;
    FSkeletalMeshRenderPass* SkeletalMeshRenderPass = nullptr;
    FWorldBillboardRenderPass* WorldBillboardRenderPass = nullptr;
    FEditorBillboardRenderPass* EditorBillboardRenderPass = nullptr;
    FGizmoRenderPass* GizmoRenderPass = nullptr;
    FUpdateLightBufferPass* UpdateLightBufferPass = nullptr;
    FLineRenderPass* LineRenderPass = nullptr;
    FFogRenderPass* FogRenderPass = nullptr;
    FCameraEffectRenderPass* CameraEffectRenderPass = nullptr;
    FEditorRenderPass* EditorRenderPass = nullptr;

    FCascadeParticleRenderPass* CascadeParticleRenderPass = nullptr;
    
    FDepthPrePass* DepthPrePass = nullptr;
    FTileLightCullingPass* TileLightCullingPass = nullptr;
    FLightHeatMapRenderPass* LightHeatMapRenderPass = nullptr;

    FCompositingPass* CompositingPass = nullptr;
    FPostProcessCompositingPass* PostProcessCompositingPass = nullptr;
    
    FSlateRenderPass* SlateRenderPass = nullptr;
};

template<typename T>
inline ID3D11Buffer* FRenderer::CreateImmutableVertexBuffer(const FString& key, const TArray<T>& Vertices)
{
    FVertexInfo VertexBufferInfo;
    BufferManager->CreateVertexBuffer(key, Vertices, VertexBufferInfo);
    return VertexBufferInfo.VertexBuffer;
}

inline ID3D11Buffer* FRenderer::CreateImmutableIndexBuffer(const FString& key, const TArray<uint32>& indices)
{
    FIndexInfo IndexInfo;
    BufferManager->CreateIndexBuffer(key, indices, IndexInfo);
    return IndexInfo.IndexBuffer;
}
