#pragma once

class UParticleSystem;
class UParticleSystemComponent;
class UWorld;
class FEditorViewportClient;
class AParticleActor;

class FParticlePreviewController
{

public:
    FParticlePreviewController(UWorld* InWorld, FEditorViewportClient* InViewport)
        : PreviewWorld(InWorld)
        , AttachedViewport(InViewport)
    {
    }

    void Initialize(UParticleSystemComponent* InParticleSystemComponent);

    UParticleSystemComponent* GetParticleSystemComponent() const;
    FEditorViewportClient* GetViewportClient() const { return AttachedViewport; }

    void Release();

private:
    UWorld* PreviewWorld = nullptr;
    FEditorViewportClient* AttachedViewport = nullptr;

    UParticleSystem* TargetParticleSystem = nullptr;
    UParticleSystemComponent* PreviewParticleSystemComponent = nullptr;
    AParticleActor* PreviewActor = nullptr;
};
