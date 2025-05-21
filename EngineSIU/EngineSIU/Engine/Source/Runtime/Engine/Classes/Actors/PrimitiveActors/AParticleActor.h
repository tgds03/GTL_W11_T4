#pragma once

#include "GameFramework/Actor.h"

class UParticleSystemComponent;

class AParticleActor : public AActor
{
    DECLARE_CLASS(AParticleActor, AActor)
public:
    AParticleActor();

protected:
    UPROPERTY
    (UParticleSystemComponent*, ParticleSystemComponent, = nullptr);

};
