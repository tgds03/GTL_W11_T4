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
    ParticleSystemComponent->Template = ParticleSystem;


    UParticleSpriteEmitter* ParticleSpriteEmitter = FObjectFactory::ConstructObject<UParticleSpriteEmitter>(nullptr);
    ParticleSystem->Emitters.Add(ParticleSpriteEmitter);
    ParticleSpriteEmitter->CreateLODLevel(0);
    ParticleSpriteEmitter->SetToSensibleDefaults();

    ParticleSpriteEmitter->GetLODLevel(0)->InsertModule(UParticleModuleTypeDataMesh::StaticClass(), ParticleSpriteEmitter);
    
    SetActorTickInEditor(true);
}
