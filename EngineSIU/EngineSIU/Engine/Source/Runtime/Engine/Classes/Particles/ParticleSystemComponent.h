#pragma once
#include "Components/PrimitiveComponent.h"
#include "UObject/ObjectMacros.h"

struct FDynamicEmitterDataBase;

class UFXSystemComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UFXSystemComponent, UPrimitiveComponent)
public:
    UFXSystemComponent() = default;
};


class UParticleSystemComponent : public UFXSystemComponent
{
    DECLARE_CLASS(UParticleSystemComponent, UFXSystemComponent)
public:

    UParticleSystemComponent() = default;


    virtual void TickComponent(float DeltaTime) override;
    /** Possibly parallel phase of TickComponent **/
    void ComputeTickComponent_Concurrent();
    
    class UParticleSystem* Template;

public:
    TArray<struct FParticleEmitterInstance*> EmitterInstances;

    // mutable TArray<FDynamicEmitterDataBase*> DynamicDataForThisFrame;
    TArray<FDynamicEmitterDataBase*> EmitterRenderData;
};
