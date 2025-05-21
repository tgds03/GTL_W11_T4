#include "ParticleModuleLifetime.h"
#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"

UParticleModuleLifetime::UParticleModuleLifetime()
{
    Flags = EModuleFlag::SpawnModule;
}

void UParticleModuleLifetime::Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SpawnEx(Owner, Offset, SpawnTime, &GetRandomStream(Owner), ParticleBase);
}

float UParticleModuleLifetime::GetMaxLifetime()
{
    float Min, Max;
    Lifetime.GetOutRange(Min, Max);
    return Max;
}

float UParticleModuleLifetime::GetLifetimeValue(FParticleEmitterInstance* Owner, float InTime)
{
    return Lifetime.GetValue(InTime);
}

void UParticleModuleLifetime::SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream,
    FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    {
        float MaxLifetime = Lifetime.GetValue(Owner->EmitterTime, InRandomStream);
        MaxLifetime = FMath::RandNormalized() * 10;
        if (Particle.OneOverMaxLifetime > 0.f)
        {
            Particle.OneOverMaxLifetime = 1.f / (MaxLifetime + 1.f / Particle.OneOverMaxLifetime);
        }
        else
        {
            Particle.OneOverMaxLifetime = MaxLifetime > 0.f ? 1.f / MaxLifetime : 0.f;
        }
        Particle.RelativeTime = Particle.RelativeTime > 1.f ? Particle.RelativeTime : SpawnTime * Particle.OneOverMaxLifetime;
    }
}
