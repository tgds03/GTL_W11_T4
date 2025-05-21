#include "ParticleModuleVelocity.h"

#include "ParticleEmitter.h"
#include "ParticleEmitterInstances.h"
#include "ParticleHelper.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "ParticleSystemComponent.h"

UParticleModuleVelocity::UParticleModuleVelocity()
{
    Flags = EModuleFlag::SpawnModule;
}

void UParticleModuleVelocity::Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SpawnEx(Owner, Offset, SpawnTime, &GetRandomStream(Owner), ParticleBase);
}

void UParticleModuleVelocity::SpawnEx(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FRandomStream* InRandomStream,
    FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    {
        FVector Vel = StartVelocity.GetValue(Owner->EmitterTime, InRandomStream);
        Vel = FVector(FMath::RandNormalized(), FMath::RandNormalized(), FMath::RandNormalized());
        FVector FromOrigin = (Particle.Location - Owner->EmitterToSimulation.GetTranslationVector()).GetSafeNormal();

        FVector OwnerScale = FVector::OneVector;
        if ((bApplyOwnerScale == true) && Owner->Component)
        {
            OwnerScale = Owner->Component->GetRelativeScale3D();
        }

        UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);
        if (LODLevel && LODLevel->RequiredModule->bUseLocalSpace)
        {
            if (bInWorldSpace == true)
            {
                Vel = FMatrix::TransformVector(Vel, FMatrix::Inverse(Owner->EmitterToSimulation));
            }
            else
            {
                Vel = FMatrix::TransformVector(Vel, Owner->EmitterToSimulation);
            }
        }
        else if (bInWorldSpace == false)
        {
            Vel = FMatrix::TransformVector(Vel, Owner->EmitterToSimulation);
        }
        Vel = Vel * OwnerScale;
        Vel += FromOrigin * StartVelocityRadial.GetValue(Owner->EmitterTime, InRandomStream) * OwnerScale;
        Particle.Velocity += Vel;
        Particle.BaseVelocity += Vel;
    }
}
