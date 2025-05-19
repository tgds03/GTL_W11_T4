#include "ParticleModuleSpawn.h"
#include "Distribution/Distribution.h"

bool UParticleModuleSpawn::GetSpawnAmount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover, float DeltaTime, int32& Number,
    float& Rate)
{
    return false;
}

float UParticleModuleSpawn::GetMaximumSpawnRate()
{
    float MinSpawn, MaxSpawn;
    float MinScale, MaxScale;

    Rate.GetOutRange(MinSpawn, MaxSpawn);
    RateScale.GetOutRange(MinScale, MaxScale);
    return MaxSpawn * MaxScale;
}
