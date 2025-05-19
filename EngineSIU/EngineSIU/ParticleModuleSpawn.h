#pragma once

#include "UObject/ObjectMacros.h"
#include "Particles/ParticleEmitter.h"
#include "ParticleModuleSpawnBase.h"
#include "EnumAsByte.h"

// FIX-ME: Should implement PCH.h
#define WITH_EDITOR 1

class UParticleLODLevel;
struct FRawDistributionFloat;

class UParticleModuleSpawn : public UParticleModuleSpawnBase
{
public:
    /** The rate at which to spawn particles. */
    FRawDistributionFloat Rate;

    /** The scalar to apply to the rate. */
    FRawDistributionFloat RateScale;

    /** The array of burst entries. */
    TArray<FParticleBurst> BurstList;

    /** Scale all burst entries by this amount. */
    FRawDistributionFloat BurstScale;

    /** The method to utilize when burst-emitting particles. */
    TEnumAsByte<EParticleBurstMethod> ParticleBurstMethod;

    /**	If true, the SpawnRate will be scaled by the global CVar r.EmitterSpawnRateScale */
    uint32 bApplyGlobalSpawnRateScale : 1;

    /** Initializes the default values for this property */
    void InitializeDefaults();

    //~ Begin UObject Interface
#if WITH_EDITOR
    virtual void	PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
    virtual void	PostInitProperties() override;
    virtual void	PostLoad() override;
    //~ End UObject Interface

    //~ Begin UParticleModule Interface
    virtual bool	GenerateLODModuleValues(UParticleModule* SourceModule, float Percentage, UParticleLODLevel* LODLevel) override;
    //~ End UParticleModule Interface

    //~ Begin UParticleModuleSpawnBase Interface
    virtual bool GetSpawnAmount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover,
        float DeltaTime, int32& Number, float& Rate) override;
    virtual float GetMaximumSpawnRate() override;
    virtual float GetEstimatedSpawnRate() override;
    virtual int32 GetMaximumBurstCount() override;
    //~ End UParticleModuleSpawnBase Interface

    float GetGlobalRateScale()const;
};



