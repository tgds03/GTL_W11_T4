#pragma once
#include "ParticleModule.h"
#include "Distribution/Distribution.h"

class UParticleModuleSpawn: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleSpawn, UParticleModule)
public:
    UParticleModuleSpawn() = default;

    FDistributionFloat Rate;
    FDistributionFloat RateScale;

    virtual bool GetSpawnAmount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover, float DeltaTime, int32& Number, float& Rate);
    virtual float GetMaximumSpawnRate();
};
