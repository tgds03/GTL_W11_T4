#include "AParticleActor.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"

AParticleActor::AParticleActor()
{
    ParticleSystemComponent = AddComponent<UParticleSystemComponent>();
    RootComponent = ParticleSystemComponent;

    FResourceManager::CreateStaticMesh("Contents/Reference/Reference.obj");
    FDynamicMeshEmitterData* TempEmitterData = new FDynamicMeshEmitterData(nullptr);
    TempEmitterData->StaticMesh = FResourceManager::GetStaticMesh(L"Contents/Reference/Reference.obj");
    TempEmitterData->Source = FDynamicMeshEmitterReplayData();
    TempEmitterData->Source.ActiveParticleCount = 100;
    TempEmitterData->Source.MaxDrawCount = 100;
    
    ParticleSystemComponent->TempTestEmitterRenderData.Add(TempEmitterData);
}
