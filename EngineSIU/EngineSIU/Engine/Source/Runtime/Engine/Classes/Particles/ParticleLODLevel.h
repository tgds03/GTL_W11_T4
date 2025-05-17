#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "ParticleEmitterInstances.h"

class UParticleLODLevel : public UObject
{
    DECLARE_CLASS(UParticleLODLevel, UObject)
public:
    UParticleLODLevel() = default;
    
public:
    /** The optional EventGenerator module. */
    class UParticleModuleEventGenerator* EventGenerator;

    class UParticleModuleRequired* RequiredModule;
    class UParticleModuleSpawn* SpawnModule;
    TArray<class UParticleModule*> Modules;
    TArray<class UParticleModule*> SpawnModules;
    TArray<class UParticleModule*> UpdateModules;
    TArray<class UParticleModule*> FinalUpdateModules;
};
