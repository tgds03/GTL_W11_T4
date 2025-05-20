#include "AParticleActor.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSpriteEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleModuleSpawn.h"
// #include "Particles/TypeData/ParticleModuleTypeDataMesh.h"

AParticleActor::AParticleActor()
{
    ParticleSystemComponent = AddComponent<UParticleSystemComponent>();
    RootComponent = ParticleSystemComponent;
    
    UParticleSystem* ParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    {
        ParticleSystem->Delay = 1;
        ParticleSystem->DelayLow = 0;
        ParticleSystem->bUseDelayRange = false;

        ParticleSystem->WarmupTime = 0;
        ParticleSystem->WarmupTickRate = 30;
    }
    ParticleSystemComponent->Template = ParticleSystem;


    UParticleSpriteEmitter* ParticleSpriteEmitter = FObjectFactory::ConstructObject<UParticleSpriteEmitter>(nullptr);

    ParticleSpriteEmitter->CreateLODLevel(0);
    ParticleSystem->UpdateAllModuleLists();

    ParticleSystem->Emitters.Add(ParticleSpriteEmitter);

    // UParticleModuleTypeDataMesh* ParticleModuleTypeDataMesh = FObjectFactory::ConstructObject<UParticleModuleTypeDataMesh>(nullptr);
    // ParticleLODLevel->TypeDataModule = ParticleModuleTypeDataMesh;
    // ParticleLODLevel->Modules.Add(ParticleModuleTypeDataMesh);

    ParticleSpriteEmitter->CacheEmitterModuleInfo();
    
    
    {
        FDynamicSpriteEmitterData* TempEmitterData = new FDynamicSpriteEmitterData(nullptr);
        TempEmitterData->Source = FDynamicSpriteEmitterReplayData();
        TempEmitterData->Source.ActiveParticleCount = 100;
        TempEmitterData->Source.MaxDrawCount = 100;
    
        ParticleSystemComponent->TempTestEmitterRenderData.Add(TempEmitterData);
    }
    
    {
        FResourceManager::CreateStaticMesh("Contents/Reference/Reference.obj");
        FDynamicMeshEmitterData* TempEmitterData = new FDynamicMeshEmitterData(nullptr);
        TempEmitterData->StaticMesh = FResourceManager::GetStaticMesh(L"Contents/Reference/Reference.obj");
        TempEmitterData->Source = FDynamicMeshEmitterReplayData();
        TempEmitterData->Source.ActiveParticleCount = 100;
        TempEmitterData->Source.MaxDrawCount = 100;
        
        ParticleSystemComponent->TempTestEmitterRenderData.Add(TempEmitterData);
    }
    
    SetActorTickInEditor(true);
}
