#pragma once
#include "IRenderPass.h"
#include "Container/Array.h"

class UParticleSystemComponent;

class FCascadeParticleRenderPass : public IRenderPass
{
public:
    FCascadeParticleRenderPass() = default;
    virtual ~FCascadeParticleRenderPass() override;
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    
    virtual void PrepareRenderArr() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;

private:
    void CreateShader();
    void ReleaseShader();
    
    void PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void RenderParticles(const std::shared_ptr<FEditorViewportClient>& Viewport) const;

private:
    TArray<UParticleSystemComponent*> ParticleSystemComponents;

    
    struct ID3D11SamplerState* SamplerState;
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
};
