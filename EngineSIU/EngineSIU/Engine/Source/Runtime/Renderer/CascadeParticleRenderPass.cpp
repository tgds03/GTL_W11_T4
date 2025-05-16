#include "CascadeParticleRenderPass.h"

#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/UObjectIterator.h"

FCascadeParticleRenderPass::~FCascadeParticleRenderPass()
{
    ReleaseShader();
}

void FCascadeParticleRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
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
    PrepareRenderState();

    RenderParticles();

    ReleaseResources();
}

void FCascadeParticleRenderPass::ClearRenderArr()
{
    ParticleSystemComponents.Empty();
}

void FCascadeParticleRenderPass::CreateShader()
{
}

void FCascadeParticleRenderPass::ReleaseShader()
{
}

void FCascadeParticleRenderPass::PrepareRenderState()
{
}

void FCascadeParticleRenderPass::RenderParticles()
{
}

void FCascadeParticleRenderPass::ReleaseResources()
{
}
