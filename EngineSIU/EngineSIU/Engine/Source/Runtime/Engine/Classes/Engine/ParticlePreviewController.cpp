#include "ParticlePreviewController.h"
#include "Actors/PrimitiveActors/AParticleActor.h"
#include "Particles/ParticleEmitter.h"
#include "World/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

void FParticlePreviewController::Initialize(UParticleSystem* InParticleSystem)
{
    if (!InParticleSystem)
    {
        return;
    }
    TargetParticleSystem = InParticleSystem;

    PreviewActor = PreviewWorld->SpawnActor<AParticleActor>();
    PreviewActor->SetActorLabel(TEXT("OBJ_PARTICLE"));
    PreviewParticleSystemComponent = PreviewActor->GetComponentByClass<UParticleSystemComponent>();

    auto Emitters = PreviewParticleSystemComponent->Template->Emitters;
    for (auto& Emitter : Emitters)
    {
        PreviewParticleSystemComponent->Template->DeleteEmitter(Emitter);
    }
    GUObjectArray.MarkRemoveObject(PreviewParticleSystemComponent->Template);
    
    PreviewParticleSystemComponent->SetParticleSystem(TargetParticleSystem);
}

void FParticlePreviewController::Release()
{
    PreviewActor = nullptr;
    PreviewParticleSystemComponent = nullptr;
    TargetParticleSystem = nullptr;
    PreviewWorld = nullptr;
    AttachedViewport = nullptr;
}
