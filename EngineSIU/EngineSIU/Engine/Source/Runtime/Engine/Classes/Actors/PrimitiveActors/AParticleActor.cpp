#include "AParticleActor.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSpriteEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleModuleVelocity.h"

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
    ParticleSystem->Emitters.Add(ParticleSpriteEmitter);
    ParticleSpriteEmitter->CreateLODLevel(0);

    
    UParticleLODLevel* ParticleLodLevel = ParticleSpriteEmitter->GetLODLevel(0);
    ParticleLodLevel->InsertModule(UParticleModuleVelocity::StaticClass(), ParticleSpriteEmitter);
    
    
    
    // TODO 빼버리기 아마 빼도 될거임. 아마? InsertModule 한번이라도 실행하면?
    ParticleSystem->UpdateAllModuleLists();
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
