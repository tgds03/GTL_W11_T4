#include "AParticleActor.h"

#include "ParticleHelper.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSpriteEmitter.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"

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
    ParticleSpriteEmitter->SetToSensibleDefaults();

    ParticleSpriteEmitter->GetLODLevel(0)->InsertModule(UParticleModuleTypeDataMesh::StaticClass(), ParticleSpriteEmitter);
    
    SetActorTickInEditor(true);
}
