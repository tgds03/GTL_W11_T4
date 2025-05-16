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

    /** SpawnModules - These are called when particles are spawned.						*/
    TArray<class UParticleModule*> SpawnModules;
};
