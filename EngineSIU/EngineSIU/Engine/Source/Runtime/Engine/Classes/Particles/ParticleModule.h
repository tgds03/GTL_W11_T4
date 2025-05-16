#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

struct FBaseParticle;
struct FParticleEmitterInstance;

class UParticleModule : public UObject
{
    DECLARE_CLASS(UParticleModule, UObject)
public:
    UParticleModule() = default;

    /**
     *	Called on a particle that is freshly spawned by the emitter.
     *	
     *	@param	Owner		The FParticleEmitterInstance that spawned the particle.
     *	@param	Offset		The modules offset into the data payload of the particle.
     *	@param	SpawnTime	The time of the spawn.
     */
    // virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase);
};
