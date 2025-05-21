#include "ParticlePreviewController.h"

void FParticlePreviewController::Initialize(UParticleSystem* InParticleSystem)
{
}

UParticleSystemComponent* FParticlePreviewController::GetParticleSystemComponent() const
{
    return PreviewParticleSystemComponent;
}

void FParticlePreviewController::Release()
{
    PreviewActor = nullptr;
    PreviewParticleSystemComponent = nullptr;
    TargetParticleSystem = nullptr;
    PreviewWorld = nullptr;
    AttachedViewport = nullptr;
}
