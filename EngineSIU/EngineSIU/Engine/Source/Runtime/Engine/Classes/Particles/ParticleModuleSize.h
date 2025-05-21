#pragma once
#include "ParticleModule.h"
#include "Distribution/Distribution.h"

class UParticleModuleSize: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleSize, UParticleModule)
public:
    UParticleModuleSize();

    FDistributionVectorUniform StartSize;
    virtual void Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

protected:
    void SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream, FBaseParticle* ParticleBase);
};
