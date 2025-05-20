#include "ParticleModuleLocation.h"
#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"

UParticleModuleLocation::UParticleModuleLocation()
{
    Flags = EModuleFlag::SpawnModule;
    DistributeOverNPoints = 0.f;
}

void UParticleModuleLocation::InitializeDefaults()
{
}

void UParticleModuleLocation::Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SpawnEx(Owner, Offset, SpawnTime, &GetRandomStream(Owner), ParticleBase);
}

void UParticleModuleLocation::SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream,
    FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    
    FVector LocationOffset;
    if ( (DistributeOverNPoints != 0.f) && (DistributeOverNPoints != 1.f) )
    {
        float RandomNum = InRandomStream->FRand() * (Owner->EmitterTime - FMath::FloorToInt(Owner->EmitterTime));
        if (RandomNum > DistributeThreshold)
        {
            LocationOffset = StartLocation.GetValue(Owner->EmitterTime, InRandomStream);
        }
        else
        {
            FVector Min, Max;
            StartLocation.GetOutRange(Min, Max);
            float Interp = FMath::FloorToInt( InRandomStream->FRand() * (DistributeOverNPoints - 1.f) + 0.5f) / (DistributeOverNPoints - 1.f);
            FVector Lerped = FMath::Lerp(Min, Max, Interp);
            LocationOffset = Lerped;
        }
    } else
    {
        LocationOffset = StartLocation.GetValue(Owner->EmitterTime, InRandomStream);
    }

    LocationOffset = FMatrix::TransformVector(LocationOffset, Owner->EmitterToSimulation);
    Particle.Location += LocationOffset;
}
