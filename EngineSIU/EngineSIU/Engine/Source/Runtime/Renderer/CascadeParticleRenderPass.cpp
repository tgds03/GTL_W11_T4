#include "CascadeParticleRenderPass.h"

#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"
#include "ParticleHelper.h"
#include "UnrealClient.h"
#include "Asset/StaticMeshAsset.h"
#include "Components/Mesh/StaticMeshRenderData.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealEd/EditorViewportClient.h"

FCascadeParticleRenderPass::~FCascadeParticleRenderPass()
{
    ReleaseShader();
}

void FCascadeParticleRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;
    CreateShader();
}

void FCascadeParticleRenderPass::PrepareRenderArr()
{
    for (const auto iter : TObjectRange<UParticleSystemComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (iter->GetOwner() && !iter->GetOwner()->IsHidden())
            {
                ParticleSystemComponents.Add(iter);
            }
        }
    }
}

void FCascadeParticleRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    PrepareRenderState(Viewport);

    RenderParticles(Viewport);

    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FCascadeParticleRenderPass::ClearRenderArr()
{
    ParticleSystemComponents.Empty();
}

void FCascadeParticleRenderPass::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC MeshParticleLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"RELATIVE_TIME", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    };

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"MeshParticleVertexShader", L"Shaders/MeshParticleVertexShader.hlsl", "mainVS", MeshParticleLayoutDesc, ARRAYSIZE(MeshParticleLayoutDesc));
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Mesh Particle Layout & Vertex Shader - Create Error", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    hr = ShaderManager->AddPixelShader(L"MeshParticlePixelShader", L"Shaders/MeshParticlePixelShader.hlsl", "mainPS");

    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Mesh Particle Pixel Shader - Create Error", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
}

void FCascadeParticleRenderPass::ReleaseShader()
{
}

void FCascadeParticleRenderPass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FViewportResource* ViewportResource = Viewport->GetViewportResource();

    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(EResourceType::ERT_Scene);
    
    Graphics->DeviceContext->RSSetViewports(1, &ViewportResource->GetD3DViewport());
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, ViewportResource->GetDepthStencil(EResourceType::ERT_Scene)->DSV);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void FCascadeParticleRenderPass::RenderParticles(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    for (auto& ParticleSystemComponent : ParticleSystemComponents)
    {
        // FParticleDynamicData* DynamicData = ParticleSystemComponent->GetDynamicData();
        // for (FDynamicEmitterDataBase* DynamicEmitterData : DynamicData->DynamicEmitterDataArray)
        for (FDynamicEmitterDataBase* DynamicEmitterData : ParticleSystemComponent->TempTestEmitterRenderData)
        {
            if (FDynamicSpriteEmitterData* DynamicSpriteEmitterData = dynamic_cast<FDynamicSpriteEmitterData*>(DynamicEmitterData))
            {
                int32 InstanceCount = DynamicSpriteEmitterData->GetSource().ActiveParticleCount;
                const FDynamicSpriteEmitterReplayData& Source = (const FDynamicSpriteEmitterReplayData&)DynamicSpriteEmitterData->GetSource();
                if ((Source.MaxDrawCount >= 0) && (Source.ActiveParticleCount > Source.MaxDrawCount))
                {
                    InstanceCount = Source.MaxDrawCount;
                }
                
                // if (DynamicSpriteEmitterData->sort)
                // {
                //     DynamicSpriteEmitterData->Sort();
                // }
                TArray<FParticleSpriteVertex> InstanceData;
                InstanceData.SetNum(InstanceCount);
                // 각 파티클 개체(각 입자)의 Instnace 데이터를 얻음. (Per Particle)
                // DynamicSpriteEmitterData->GetVertexAndIndexData(InstanceData.GetData(), ParticleOrder, Viewport->GetCameraLocation(), ParticleSystemComponent->GetWorldMatrix());
            
                UINT Offset = 0;
                
                // UINT InstanceStride = (UINT)DynamicSpriteEmitterData->GetDynamicVertexStride();
                UINT InstanceOffset = 0;
            
                FVertexInfo VertexInfo;
                FIndexInfo IndexInfo;
                BufferManager->GetQuadBuffer(VertexInfo, IndexInfo);
                
                Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexInfo.VertexBuffer, &VertexInfo.Stride, &Offset);
            
            
                FString ParticleSystemVertexBufferName = ParticleSystemComponent->GetName() + FString::FromInt(ParticleSystemComponent->GetUUID());
                FVertexInfo ParticleVertexInfo;
                BufferManager->CreateVertexBuffer(ParticleSystemVertexBufferName, InstanceData, ParticleVertexInfo);
                BufferManager->UpdateDynamicVertexBuffer(ParticleSystemVertexBufferName, InstanceData);
                Graphics->DeviceContext->IASetVertexBuffers(1, 1, &ParticleVertexInfo.VertexBuffer, &ParticleVertexInfo.Stride, &InstanceOffset);
                //Graphics->DeviceContext->IASetVertexBuffers(1, 1, &InstanceBuffer, &InstanceStride, &InstanceOffset);
            
            
                Graphics->DeviceContext->IASetIndexBuffer(IndexInfo.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
            
            
                // TODO Set Material
                
                Graphics->DeviceContext->DrawIndexedInstanced(IndexInfo.NumIndices, InstanceCount, 0, 0, 0);
            }
            else if (FDynamicMeshEmitterData* DynamicMeshEmitterData = dynamic_cast<FDynamicMeshEmitterData*>(DynamicEmitterData))
            {
                ID3D11InputLayout* InputLayout = ShaderManager->GetInputLayoutByKey(L"MeshParticleVertexShader");
                ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"MeshParticlePixelShader");
                ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"MeshParticleVertexShader");

                Graphics->DeviceContext->IASetInputLayout(InputLayout);
                Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
                Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
                
                const FDynamicMeshEmitterReplayData& Source = (const FDynamicMeshEmitterReplayData&)DynamicMeshEmitterData->GetSource();
                int32 InstanceCount = Source.ActiveParticleCount;
                if ((Source.MaxDrawCount >= 0) && (Source.ActiveParticleCount > Source.MaxDrawCount))
                {
                    InstanceCount = Source.MaxDrawCount;
                }
                FStaticMeshRenderData* RenderData = DynamicMeshEmitterData->StaticMesh->GetRenderData();
                
                UINT Offset = 0;
                UINT InstanceOffset = 0;
                
                FVertexInfo VertexInfo;
                // Instance + UUID
                BufferManager->CreateVertexBuffer(RenderData->ObjectName, RenderData->Vertices, VertexInfo);
                Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexInfo.VertexBuffer, &VertexInfo.Stride, &Offset);

                TArray<FMeshParticleInstanceVertex> InstanceData;
                InstanceData.SetNum(InstanceCount);
                for (auto& Instance : InstanceData)
                {
                    Instance.Color = FLinearColor::Red;
                    
                    {
                        Instance.Transform[0] = FVector4::ZeroVector;
                        Instance.Transform[0].W = ParticleSystemComponent->GetWorldLocation().X + FMath::RandHelper(100);
                    }

                    {
                        Instance.Transform[1] = FVector4::ZeroVector;
                        Instance.Transform[1].W = ParticleSystemComponent->GetWorldLocation().Y + FMath::RandHelper(100);
                    }
                    
                    {
                        Instance.Transform[2] = FVector4::ZeroVector;
                        Instance.Transform[2].W = ParticleSystemComponent->GetWorldLocation().Z + FMath::RandHelper(100);
                    }
                    
                    
                    Instance.Transform[0].X = 2;
                    Instance.Transform[1].Y = 2;
                    Instance.Transform[2].Z = 2;
                    auto randomValueX = FMath::RandHelper(10);
                    auto randomValueY = FMath::RandHelper(10);
                    auto randomValueZ = FMath::RandHelper(10);
                    Instance.Velocity = FVector(randomValueX, randomValueY, randomValueZ);
                    Instance.RelativeTime = 0.5f;
                } 

                // TODO
                // 각 파티클 개체(각 입자)의 Instnace 데이터를 얻음. (Per Particle)
                // DynamicMeshEmitterData->GetInstanceData(InstanceData.GetData(), ParticleSystemComponent->GetWorldMatrix());
                
                // VertexBuffer 구조체로 들어오는 MainVs -> VSINPUT 구조체에 이어 붙이면 됨.
                FString ParticleSystemVertexBufferName = ParticleSystemComponent->GetName() + FString::FromInt(ParticleSystemComponent->GetUUID());
                FVertexInfo ParticleVertexInfo;
                BufferManager->CreateVertexBuffer(ParticleSystemVertexBufferName, InstanceData, ParticleVertexInfo, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
                BufferManager->UpdateDynamicVertexBuffer(ParticleSystemVertexBufferName, InstanceData);
                Graphics->DeviceContext->IASetVertexBuffers(1, 1, &ParticleVertexInfo.VertexBuffer, &ParticleVertexInfo.Stride, &InstanceOffset);
                
                FIndexInfo ParticleIndexInfo;
                BufferManager->CreateIndexBuffer(RenderData->ObjectName, RenderData->Indices, ParticleIndexInfo);
                Graphics->DeviceContext->IASetIndexBuffer(ParticleIndexInfo.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

                // TODO What
                // BufferManager->UpdateDynamicIndexBuffer(ParticleSystemVertexBufferName, );


                // TODO Material Section에 따라 분리 IndexCount, StartLocation 설정으로
                // Set Material, Set Section, Set Sampler, Set PSResources

                for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
                {
                    // uint32 MaterialIndex = RenderData->MaterialSubsets[SubMeshIndex].MaterialIndex;
                    // FSubMeshConstants SubMeshData = FSubMeshConstants(false);

                    // if (OverrideMaterials[MaterialIndex] != nullptr)
                    // {
                    //     MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[MaterialIndex]->GetMaterialInfo());
                    // }
                    // else
                    // {
                    //     MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[MaterialIndex]->Material->GetMaterialInfo());
                    // }
                    
                    uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
                    uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;
                    Graphics->DeviceContext->DrawIndexedInstanced(IndexCount, InstanceCount, StartIndex, 0, 0);
                }
            }
        }
    }
}
