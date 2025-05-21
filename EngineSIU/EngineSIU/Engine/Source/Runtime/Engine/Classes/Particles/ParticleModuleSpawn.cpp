#include "ParticleModuleSpawn.h"
#include "Distribution/Distribution.h"

UParticleModuleSpawnBase::UParticleModuleSpawnBase() : Super()
{
    bProcessSpawnRate = true;
}

UParticleModuleSpawn::UParticleModuleSpawn() : Super()
{
    bProcessSpawnRate = true;
    // LODDuplicate = false;
    bApplyGlobalSpawnRateScale = true;
    Rate.Constant = 20.f;
    RateScale.Constant = 1.f;
}

void UParticleModuleSpawn::InitializeDefaults()
{
    // if(!Rate.IsCreated())
    // {
    //     FDistributionFloatConstant* RequiredDistributionSpawnRate = NewObject<FDistributionFloatConstant>(this, TEXT("RequiredDistributionSpawnRate"));
    //     RequiredDistributionSpawnRate->Constant = 20.0f;
    //     Rate.Distribution = RequiredDistributionSpawnRate;
    // }
    //
    // if(!RateScale.IsCreated())
    // {
    //     FDistributionFloatConstant* RequiredDistributionSpawnRateScale = NewObject<FDistributionFloatConstant>(this, TEXT("RequiredDistributionSpawnRateScale"));
    //     RequiredDistributionSpawnRateScale->Constant = 1.0f;
    //     RateScale.Distribution = RequiredDistributionSpawnRateScale;
    // }
}

bool UParticleModuleSpawn::GetSpawnAmount(FParticleEmitterInstance* Owner, int32 Offset, float OldLeftover, float DeltaTime, int32& Number,
                                          float& Rate)
{
    assert(Owner);
    return false;
}

float UParticleModuleSpawn::GetMaximumSpawnRate()
{
    return Rate.Constant * RateScale.Constant;
}

float UParticleModuleSpawn::GetEstimatedSpawnRate()
{
    return UParticleModuleSpawnBase::GetEstimatedSpawnRate();
}

float UParticleModuleSpawn::GetGlobalRateScale() const
{
    return 1.0f;
    // static const auto EmitterRateScaleCVar = IConsoleManager::Get().FindTConsoleVariableDataFloat(TEXT("r.EmitterSpawnRateScale"));
    // return (bApplyGlobalSpawnRateScale && EmitterRateScaleCVar) ? EmitterRateScaleCVar->GetValueOnAnyThread() : 1.0f;
}
