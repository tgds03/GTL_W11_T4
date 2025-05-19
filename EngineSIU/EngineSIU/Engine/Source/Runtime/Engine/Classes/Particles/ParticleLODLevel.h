#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "ParticleEmitterInstances.h"

class UParticleModuleTypeDataBase;
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

    TArray<class UParticleModule*> Modules;
    /** The required module for this LOD level											*/
    UParticleModuleRequired* RequiredModule;
    UParticleModuleSpawn* SpawnModule;
    TArray<class UParticleModule*> SpawnModules;
    UParticleModuleTypeDataBase* TypeDataModule;
    TArray<class UParticleModule*> UpdateModules;
    TArray<class UParticleModule*> FinalUpdateModules;
    /** True if the LOD level is enabled, meaning it should be updated and rendered.	*/
    uint8 bEnabled : 1;
    uint8 bUseLocalSpace: 1;

    int32 PeakActiveParticles;


};
