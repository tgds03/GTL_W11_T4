#pragma once
#include "ParticleModule.h"
#include "Distribution/Distribution.h"

class UParticleModuleLocation: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleLocation, UParticleModule)
public:
    UParticleModuleLocation();

    FDistributionVector StartLocation;
    float DistributeOverNPoints;
    float DistributeThreshold;

    void InitializeDefaults();

    //virtual void PostInitProperties() override;
    virtual void Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;
    // virtual void Render3DPreview(FParticleEmitterInstance* Owner, const FSceneView* View,FPrimitiveDrawInterface* PDI) override;

protected:
    virtual void SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream, FBaseParticle* ParticleBase);
};
