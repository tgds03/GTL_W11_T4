#include "ParticleModuleSize.h"

#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"

UParticleModuleSize::UParticleModuleSize()
{
    Flags = static_cast<EModuleFlag::EModuleFlags>(EModuleFlag::SpawnModule | EModuleFlag::UpdateModule);
}

void UParticleModuleSize::Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SpawnEx(Owner, Offset, SpawnTime, &GetRandomStream(Owner), ParticleBase);
}

void UParticleModuleSize::SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream,
    FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    FVector Size = StartSize.GetValue(Owner->EmitterTime, InRandomStream);
    Particle.Size += Size;
    // AdjustParticleBaseSizeForUVFlipping(Size, Owner->CurrentLODLevel->RequiredModule->UVFlippingMode, *InRandomStream);
    Particle.BaseSize += Size;
}
