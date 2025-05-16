#pragma once
#include "UObject/Object.h"

#include "ParticleEmitterInstances.h"
#include "UObject/ObjectMacros.h"

class UParticleLODLevel;

class UParticleEmitter : public UObject
{
    DECLARE_CLASS(UParticleEmitter, UObject)

public:
    UParticleEmitter() = default;

    TArray<class UParticleLODLevel*> LODLevels;

    /** Map module pointers to their offset into the particle data.		*/
    TMap<UParticleModule*, uint32> ModuleOffsetMap;

    /** Map module pointers to their offset into the instance data.		*/
    TMap<UParticleModule*, uint32> ModuleInstanceOffsetMap;
    
    /** GetCurrentLODLevel
*	Returns the currently set LODLevel. Intended for game-time usage.
*	Assumes that the given LODLevel will be in the [0..# LOD levels] range.
*	
*	@return NULL if the requested LODLevel is not valid.
*			The pointer to the requested UParticleLODLevel if valid.
*/
    //UParticleLODLevel* GetCurrentLODLevel(FParticleEmitterInstance* Instance);
};
