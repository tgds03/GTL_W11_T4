#include "AParticleActor.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSpriteEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleLifetime.h"
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
    
    ParticleLodLevel->InsertModule(UParticleModuleLifetime::StaticClass(), ParticleSpriteEmitter);

    // TODO 빼버리기 아마 빼도 될거임. 아마? InsertModule 한번이라도 실행하면?
    ParticleSystem->UpdateAllModuleLists();
    ParticleSpriteEmitter->CacheEmitterModuleInfo();
    
    SetActorTickInEditor(true);
}
