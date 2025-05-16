#pragma once
#include "Components/PrimitiveComponent.h"
#include "UObject/ObjectMacros.h"

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

    class UParticleSystem* Template;

public:
    TArray<struct FParticleEmitterInstance*> EmitterInstances;
    
};
