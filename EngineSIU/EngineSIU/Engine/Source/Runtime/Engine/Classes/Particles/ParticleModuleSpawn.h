#pragma once

#include "ParticleModuleSpawnBase.h"

#include "Distribution/Distribution.h"

// FIX-ME: Should implement PCH.h
#define WITH_EDITOR 1


class UParticleModuleSpawn : public UParticleModuleSpawnBase
{
    DECLARE_CLASS(UParticleModuleSpawn, UParticleModuleSpawnBase)
public:
    UParticleModuleSpawn();
    
    /** The rate at which to spawn particles. */
    FDistributionFloatConstant Rate;

    /** The scalar to apply to the rate. */
    FDistributionFloatConstant RateScale;
    
    /** The array of burst entries. */
    // TArray<FParticleBurst> BurstList;

    /** Scale all burst entries by this amount. */
    // FDistributionFloat BurstScale;

    /** The method to utilize when burst-emitting particles. */
    // TEnumAsByte<EParticleBurstMethod> ParticleBurstMethod;

    /**	If true, the SpawnRate will be scaled by the global CVar r.EmitterSpawnRateScale */
    uint32 bApplyGlobalSpawnRateScale : 1;

    /** Initializes the default values for this property */
    void InitializeDefaults();

    //~ Begin UParticleModule Interface
    // virtual bool	GenerateLODModuleValues(UParticleModule* SourceModule, float Percentage, UParticleLODLevel* LODLevel) override;
    //~ End UParticleModule Interface

    //~ Begin UParticleModuleSpawnBase Interface
    virtual bool GetSpawnAmount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover,
        float DeltaTime, int32& Number, float& Rate) override;
    virtual float GetMaximumSpawnRate() override;
    virtual float GetEstimatedSpawnRate() override;
    //~ End UParticleModuleSpawnBase Interface

    float GetGlobalRateScale() const;
};



