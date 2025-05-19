#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "ParticleEmitterInstances.h"

class UParticleModuleRequired;
class UParticleModuleSpawn;

class UParticleLODLevel : public UObject
{
    DECLARE_CLASS(UParticleLODLevel, UObject)
public:
    UParticleLODLevel() = default;
    
public:
    /** The index value of the LOD level												*/\
    int32 Level;

    /** The optional EventGenerator module. */
    class UParticleModuleEventGenerator* EventGenerator;

    /** SpawnModules - These are called when particles are spawned.						*/
    TArray<class UParticleModule*> SpawnModules;

    /** True if the LOD level is enabled, meaning it should be updated and rendered.	*/
    uint32 bEnabled : 1;

    /** The required module for this LOD level											*/
    UParticleModuleRequired* RequiredModule;

    int32 PeakActiveParticles;

    UParticleModuleSpawn* SpawnModule;

};
