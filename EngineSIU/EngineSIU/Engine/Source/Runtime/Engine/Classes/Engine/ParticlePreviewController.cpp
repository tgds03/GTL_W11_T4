#include "ParticlePreviewController.h"

void FParticlePreviewController::Initialize(UParticleSystem* InParticleSystem)
{
    if (!InParticleSystem)
    {
        return;
    }
    TargetParticleSystem = InParticleSystem;


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
