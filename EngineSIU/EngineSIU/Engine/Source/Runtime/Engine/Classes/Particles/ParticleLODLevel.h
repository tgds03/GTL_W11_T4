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
    /** The index value of the LOD level												*/
    int32 Level;

    /** True if the LOD level is enabled, meaning it should be updated and rendered.	*/
    uint32 bEnabled:1;

    /** The required module for this LOD level											*/
    class UParticleModuleRequired* RequiredModule;

    /** An array of particle modules that contain the adjusted data for the LOD level	*/
    TArray<class UParticleModule*> Modules;

    // Module<SINGULAR> used for emitter type "extension".
    class UParticleModuleTypeDataBase* TypeDataModule;

    /** The SpawnRate/Burst module - required by all emitters. */
    class UParticleModuleSpawn* SpawnModule;
    
    /** The optional EventGenerator module. */
    class UParticleModuleEventGenerator* EventGenerator;

    TArray<class UParticleModule*> SpawnModules;
    TArray<class UParticleModule*> UpdateModules;
    TArray<class UParticleModule*> FinalUpdateModules;
};
