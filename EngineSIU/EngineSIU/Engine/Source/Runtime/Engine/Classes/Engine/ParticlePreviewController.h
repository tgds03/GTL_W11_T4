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

    void Initialize(UParticleSystem* InParticleSystem);

    FEditorViewportClient* GetViewportClient() const { return AttachedViewport; }

    void Release();

    UParticleSystem* TargetParticleSystem = nullptr;
    AParticleActor* PreviewActor = nullptr;
    UParticleSystemComponent* PreviewParticleSystemComponent = nullptr;
private:
    UWorld* PreviewWorld = nullptr;
    FEditorViewportClient* AttachedViewport = nullptr;

};
