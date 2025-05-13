#include "SkeletalMeshRenderPass.h"

#include <array>

#include "EngineLoop.h"
#include "World/World.h"

#include "RendererHelpers.h"
#include "ShadowManager.h"
#include "ShadowRenderPass.h"
#include "UnrealClient.h"
#include "Math/JungleMath.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/SkeletalMesh/SkeletalMesh.h"


#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"

#include "PropertyEditor/ShowFlags.h"

#include "UnrealEd/EditorViewportClient.h"
#include "Components/Light/PointLightComponent.h"
#include "Contents/Actors/Fish.h"

FSkeletalMeshRenderPass::FSkeletalMeshRenderPass()
    : FMeshRenderPassBase()
{

}

FSkeletalMeshRenderPass::~FSkeletalMeshRenderPass()
{
    ReleaseShader();
}

void FSkeletalMeshRenderPass::PrepareRenderArr()
{
    for (const auto iter : TObjectRange<USkeletalMeshComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (iter->GetOwner() && !iter->GetOwner()->IsHidden())
            {
                SkeletalMeshComponents.Add(iter);
            }
        }
    }
}

void FSkeletalMeshRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ShadowManager->BindResourcesForSampling();

    PrepareRenderState(Viewport);

    RenderAllSkeletalMeshes(Viewport);

    // 렌더 타겟 해제
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Graphics->DeviceContext->PSSetShaderResources(static_cast<int>(EShaderSRVSlot::SRV_PointLight), 1, &nullSRV); // t51 슬롯을 NULL로 설정
    Graphics->DeviceContext->PSSetShaderResources(static_cast<int>(EShaderSRVSlot::SRV_DirectionalLight), 1, &nullSRV); // t51 슬롯을 NULL로 설정
    Graphics->DeviceContext->PSSetShaderResources(static_cast<int>(EShaderSRVSlot::SRV_SpotLight), 1, &nullSRV); // t51 슬롯을 NULL로 설정

    // 머티리얼 리소스 해제
    constexpr UINT NumViews = static_cast<UINT>(EMaterialTextureSlots::MTS_MAX);

    ID3D11ShaderResourceView* NullSRVs[NumViews] = { nullptr, };
    ID3D11SamplerState* NullSamplers[NumViews] = { nullptr, };

    Graphics->DeviceContext->PSSetShaderResources(0, NumViews, NullSRVs);
    Graphics->DeviceContext->PSSetSamplers(0, NumViews, NullSamplers);

    // for Gouraud shading
    ID3D11ShaderResourceView* NullSRV[1] = { nullptr, };
    ID3D11SamplerState* NullSampler[1] = { nullptr, };
    Graphics->DeviceContext->VSSetShaderResources(0, 1, NullSRV);
    Graphics->DeviceContext->VSSetSamplers(0, 1, NullSampler);

    // @todo 리소스 언바인딩 필요한가? - 답변: 네.
    // SRV 해제
    ID3D11ShaderResourceView* NullSRVs2[14] = { nullptr, };
    Graphics->DeviceContext->PSSetShaderResources(0, 14, NullSRVs2);

    // 상수버퍼 해제
    ID3D11Buffer* NullPSBuffer[9] = { nullptr, };
    Graphics->DeviceContext->PSSetConstantBuffers(0, 9, NullPSBuffer);
    ID3D11Buffer* NullVSBuffer[2] = { nullptr, };
    Graphics->DeviceContext->VSSetConstantBuffers(0, 2, NullVSBuffer);
}

void FSkeletalMeshRenderPass::ClearRenderArr()
{
    SkeletalMeshComponents.Empty();
}

void FSkeletalMeshRenderPass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    const EViewModeIndex ViewMode = Viewport->GetViewMode();

    ChangeViewMode(ViewMode);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    TArray<FString> PSBufferKeys = {
        TEXT("FLightInfoBuffer"),
        TEXT("FMaterialConstants"),
        TEXT("FLitUnlitConstants"),
        TEXT("FSubMeshConstants"),
        TEXT("FTextureConstants"),
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);
    BufferManager->BindConstantBuffer(TEXT("FDiffuseMultiplier"), 6, EShaderStage::Pixel);

    BufferManager->BindConstantBuffer(TEXT("FLightInfoBuffer"), 0, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FMaterialConstants"), 1, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FObjectConstantBuffer"), 12, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FBoneWeightConstants"), 2, EShaderStage::Vertex);


    Graphics->DeviceContext->RSSetViewports(1, &Viewport->GetViewportResource()->GetD3DViewport());

    const EResourceType ResourceType = EResourceType::ERT_Scene;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);
    FDepthStencilRHI* DepthStencilRHI = ViewportResource->GetDepthStencil(ResourceType);

    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, DepthStencilRHI->DSV);
}

void FSkeletalMeshRenderPass::UpdateBoneConstant(FSkeletonPose* SkeletonPose) const
{
    FSkeleton* Skeleton = SkeletonPose->Skeleton;

    FBoneWeightConstants BoneWeightConstants;
    for (int i = 0; i < Skeleton->BoneCount; i++) 
    {
        if (i >= MAX_BONE_NUM) 
        {
            UE_LOG(LogLevel::Warning, TEXT("Bone Constant에서 제한한 갯수 초과 "));
            break;
        }

        //BoneWeightConstants.BoneTransform[i] = Skeleton->Bones[i].InvBindTransform * Skeleton.Bones[i].GlobalTransform;
        BoneWeightConstants.BoneTransform[i] = SkeletonPose->GetSkinningMatrix(i);
    }

    BufferManager->UpdateConstantBuffer(TEXT("FBoneWeightConstants"), BoneWeightConstants);
}

void FSkeletalMeshRenderPass::RenderAllSkeletalMeshesForPointLight(const std::shared_ptr<FEditorViewportClient>& Viewport, UPointLightComponent*& PointLight)
{
    for (USkeletalMeshComponent* Comp : SkeletalMeshComponents)
    {
        if (!Comp || !Comp->GetSkeletalMesh()) { continue; }

        FSkeletalMeshRenderData* RenderData = Comp->GetSkeletalMesh()->GetRenderData();
        if (RenderData == nullptr) { continue; }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        FMatrix WorldMatrix = Comp->GetWorldMatrix();

        //ShadowRenderPass->UpdateCubeMapConstantBuffer(PointLight, WorldMatrix);
        RenderPrimitive(Comp->GetSkeletalMesh(), Comp->GetSkeletalMesh()->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());
    }
}

void FSkeletalMeshRenderPass::RenderAllSkeletalMeshes(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

    for (USkeletalMeshComponent* Comp : SkeletalMeshComponents)
    {
        if (!Comp || !Comp->GetSkeletalMesh())
        {
            continue;
        }

        FSkeletalMeshRenderData* RenderData = Comp->GetSkeletalMesh()->GetRenderData();
        if (RenderData == nullptr)
        {
            continue;
        }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();
        AActor* SelectedActor = Engine->GetSelectedActor();

        USceneComponent* TargetComponent = nullptr;

        if (SelectedComponent != nullptr)
        {
            TargetComponent = SelectedComponent;
        }
        else if (SelectedActor != nullptr)
        {
            TargetComponent = SelectedActor->GetRootComponent();
        }

        FMatrix WorldMatrix = Comp->GetWorldMatrix();
        FVector4 UUIDColor = Comp->EncodeUUID() / 255.0f;
        const bool bIsSelected = (Engine && TargetComponent == Comp);

        UpdateObjectConstant(WorldMatrix, UUIDColor, bIsSelected);

        RenderPrimitive(Comp->GetSkeletalMesh(), Comp->GetSkeletalMesh()->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());

        if (Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            FEngineLoop::PrimitiveDrawBatch.AddAABBToBatch(Comp->GetBoundingBox(), Comp->GetWorldLocation(), WorldMatrix);
        }
    }
}

void FSkeletalMeshRenderPass::RenderPrimitive(USkeletalMesh* SkeletalMesh, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const
{
    FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetRenderData();
    UpdateBoneConstant(SkeletalMesh->GetSkeletonPose());

    UINT Stride = sizeof(FSkeletalMeshVertex);
    UINT Offset = 0;

    FString KeyName(RenderData->ObjectName.c_str());
    FVertexInfo VertexInfo;

    if (FEngineLoop::IsGPUSkinningEnabled())
    {
        BufferManager->CreateDynamicVertexBuffer(KeyName, RenderData->Vertices, VertexInfo);
        BufferManager->UpdateDynamicVertexBuffer(KeyName, RenderData->Vertices);
    }
    else
    {
        BufferManager->CreateDynamicVertexBuffer(KeyName, RenderData->CPUSkinnedVertices, VertexInfo);
        BufferManager->UpdateDynamicVertexBuffer(KeyName, RenderData->CPUSkinnedVertices);
    }
   
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexInfo.VertexBuffer, &Stride, &Offset);

    FIndexInfo IndexInfo;
    BufferManager->CreateIndexBuffer(RenderData->ObjectName, RenderData->Indices, IndexInfo);
    if (IndexInfo.IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(IndexInfo.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        uint32 MaterialIndex = RenderData->MaterialSubsets[SubMeshIndex].MaterialIndex;

        FSubMeshConstants SubMeshData = (SubMeshIndex == SelectedSubMeshIndex) ? FSubMeshConstants(true) : FSubMeshConstants(false);

        BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

        if (OverrideMaterials[MaterialIndex] != nullptr)
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[MaterialIndex]->GetMaterialInfo());
        }
        else
        {
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[MaterialIndex]->Material->GetMaterialInfo());
        }

        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
    }
}

void FSkeletalMeshRenderPass::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
{
    // 현재까지는 실질적으로 FStaticMeshVetex를 사용해도 문제가 전혀 없으므로 아래 코드 그대로 사용
    // 추후 GPU SKinning과 같은 작업을 하게되면 수정 필요
    UINT Stride = sizeof(FSkeletalMeshVertex);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &Offset);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FSkeletalMeshRenderPass::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
{
    // 현재까지는 실질적으로 FStaticMeshVetex를 사용해도 문제가 전혀 없으므로 아래 코드 그대로 사용
    // 추후 GPU SKinning과 같은 작업을 하게되면 수정 필요
    UINT Stride = sizeof(FSkeletalMeshVertex);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FSkeletalMeshRenderPass::ChangeViewMode(EViewModeIndex ViewMode)
{
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;

    bool bUseGPUSkinning = FEngineLoop::IsGPUSkinningEnabled();
    std::wstring VertexShaderPrefix = bUseGPUSkinning ? L"SkeletalMesh" : L"StaticMesh";

    switch (ViewMode)
    {
        case EViewModeIndex::VMI_Lit_Gouraud:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"GOURAUD_StaticMeshPixelShader");
            UpdateLitUnlitConstant(1);
            break;
        case EViewModeIndex::VMI_Lit_Lambert:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"LAMBERT_StaticMeshPixelShader");
            UpdateLitUnlitConstant(1);
            break;
        case EViewModeIndex::VMI_Lit_BlinnPhong:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix+L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix+L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"PHONG_StaticMeshPixelShader");
            UpdateLitUnlitConstant(1);
            break;
        case EViewModeIndex::VMI_LIT_PBR:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"PBR_StaticMeshPixelShader");
            UpdateLitUnlitConstant(1);
            break;
        case EViewModeIndex::VMI_Wireframe:
        case EViewModeIndex::VMI_Unlit:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"LAMBERT_StaticMeshPixelShader");
            UpdateLitUnlitConstant(0);
            break;
        case EViewModeIndex::VMI_SceneDepth:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderDepth");
            UpdateLitUnlitConstant(0);
            break;
        case EViewModeIndex::VMI_WorldNormal:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderWorldNormal");
            UpdateLitUnlitConstant(0);
            break;
        case EViewModeIndex::VMI_WorldTangent:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShaderWorldTangent");
            UpdateLitUnlitConstant(0);
            break;
            // HeatMap ViewMode 등
        default:
            VertexShader = ShaderManager->GetVertexShaderByKey(VertexShaderPrefix + L"VertexShader");
            InputLayout = ShaderManager->GetInputLayoutByKey(VertexShaderPrefix + L"VertexShader");
            PixelShader = ShaderManager->GetPixelShaderByKey(L"LAMBERT_StaticMeshPixelShader");
            UpdateLitUnlitConstant(1);
            break;
    }

    // Rasterizer
    Graphics->ChangeRasterizer(ViewMode);

    // Setup
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
}


