#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"

bool UParticleEmitter::HasAnyEnabledLODs() const
{
    for (UParticleLODLevel* LodLevel : LODLevels)
    {
        if (LodLevel && LodLevel->bEnabled)
        {
            return true;
        }
    }

    return false;
}

FParticleEmitterInstance* UParticleEmitter::CreateInstance(UParticleSystemComponent* InComponent)
{
    UE_LOG(LogLevel::Error, TEXT("UParticleEmitter::CreateInstance is pure virtual"));
    return NULL;
}
