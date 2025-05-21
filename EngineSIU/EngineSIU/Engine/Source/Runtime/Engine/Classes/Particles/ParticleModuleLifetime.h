#pragma once
#include "ParticleModule.h"
#include "Distribution/Distribution.h"

class UParticleModuleLifetime: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleLifetime, UParticleModule)
public:
    UParticleModuleLifetime();

    FDistributionFloatUniform Lifetime;

    virtual void Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

    virtual float GetMaxLifetime();
    virtual float GetLifetimeValue(FParticleEmitterInstance* Owner, float InTime);

    void SetToSensibleDefaults(UParticleEmitter* Owner) override;
    
protected:
    void SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream, FBaseParticle* ParticleBase);
};
