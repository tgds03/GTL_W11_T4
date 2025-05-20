#include "ParticleModuleColor.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"

UParticleModuleColor::UParticleModuleColor()
{
    Flags = static_cast<EModuleFlag::EModuleFlags>(EModuleFlag::SpawnModule | EModuleFlag::UpdateModule);
    bClampAlpha = true;
}

void UParticleModuleColor::Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SpawnEx(Owner, Offset, SpawnTime, &GetRandomStream(Owner), ParticleBase);
}

void UParticleModuleColor::SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream,
    FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    {
        FVector ColorVec = StartColor.GetValue(Owner->EmitterTime, InRandomStream);
        float Alpha = StartAlpha.GetValue(Owner->EmitterTime, InRandomStream);
        Particle.Color.R = ColorVec.X;
        Particle.Color.G = ColorVec.Y;
        Particle.Color.B = ColorVec.Z;
        Particle.Color.A = Alpha;
        Particle.BaseColor = Particle.Color;
    }
}
