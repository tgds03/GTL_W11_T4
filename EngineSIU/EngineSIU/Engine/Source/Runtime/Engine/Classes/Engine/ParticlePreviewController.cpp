#include "ParticlePreviewController.h"
#include "Actors/PrimitiveActors/AParticleActor.h"
#include "World/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

void FParticlePreviewController::Initialize(UParticleSystemComponent* InParticleSystemComponent)
{
    if (!InParticleSystemComponent)
    {
        return;
    }
    TargetParticleSystem = InParticleSystemComponent->Template;

    PreviewActor = PreviewWorld->SpawnActor<AParticleActor>();
    PreviewActor->SetActorLabel(TEXT("OBJ_PARTICLE"));
    PreviewParticleSystemComponent = InParticleSystemComponent;
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
