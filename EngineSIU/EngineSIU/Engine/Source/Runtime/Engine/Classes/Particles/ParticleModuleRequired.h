#pragma once
#include "ParticleModule.h"

class UParticleModuleRequired: public UParticleModule
{
    DECLARE_CLASS(UParticleModuleRequired, UParticleModule)
public:
    UParticleModuleRequired();

    UMaterial* Material;
    FVector EmitterOrigin;
    FRotator EmitterRotation;
    int32 EmitterLoops;
    
};
