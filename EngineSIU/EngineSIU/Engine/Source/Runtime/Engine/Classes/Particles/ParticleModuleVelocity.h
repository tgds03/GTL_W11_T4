#pragma once
#include "ParticleModule.h"
#include "Distribution/Distribution.h"

class UParticleModuleVelocity: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleVelocity, UParticleModule)
public:
    UParticleModuleVelocity();

    FDistributionVector StartVelocity;

    uint8 bInWorldSpace: 1;
    uint8 bApplyOwnerScale: 1;

    virtual void Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

protected:
    virtual void SpawnEx(FParticleEmitterInstance* Owner, uint32 OFfset, float SpawnTime, FRandomStream* InRandomStream, FBaseParticle* ParticleBase);
};
