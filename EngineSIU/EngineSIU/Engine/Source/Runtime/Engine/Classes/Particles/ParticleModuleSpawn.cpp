#include "ParticleModuleSpawn.h"
#include "Distribution/Distribution.h"

UParticleModuleSpawn::UParticleModuleSpawn()
{
    bProcessSpawnRate = true;
    bApplyGlobalSpawnRateScale = true;
}

bool UParticleModuleSpawn::GetSpawnAmount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover, float DeltaTime, int32& Number,
                                          float& Rate)
{
    return false;
}

float UParticleModuleSpawn::GetMaximumSpawnRate()
{
    return Rate * RateScale;
}
