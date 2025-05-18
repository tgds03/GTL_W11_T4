#include "ParticleEmitter.h"

UParticleLODLevel* UParticleEmitter::GetCurrentLODLevel(FParticleEmitterInstance* Instance)
{
    return Instance->CurrentLODLevel;
}
