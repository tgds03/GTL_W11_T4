#pragma once
#include "ParticleModule.h"
#include "Distribution/Distribution.h"

class UParticleModuleColor: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleColor, UParticleModule)
public:
    UParticleModuleColor();

    FDistributionVectorUniform StartColor;
    FDistributionFloatUniform StartAlpha;
    uint8 bClampAlpha: 1;

    virtual void Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;
    virtual void SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream, FBaseParticle* ParticleBase);
};
