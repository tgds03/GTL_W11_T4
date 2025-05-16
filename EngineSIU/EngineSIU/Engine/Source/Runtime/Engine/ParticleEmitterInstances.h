#pragma once
#include "HAL/PlatformType.h"
#include "Math/Vector.h"

class UParticleEmitter;
class UParticleLODLevel;
class UParticleSystemComponent;

struct FParticleEmitterInstance
{
    UParticleEmitter* SpriteTemplate;

    // Owner
    UParticleSystemComponent* Component;

    int32 CurrentLODLevelIndex;
    UParticleLODLevel* CurrentLODLevel;

    /** Pointer to the particle data array.                             */
    uint8* ParticleData;
    /** Pointer to the particle index array.                            */
    uint16* ParticleIndices;
    /** Pointer to the instance data array.                             */
    uint8* InstanceData;
    /** The size of the Instance data array.                            */
    int32 InstancePayloadSize;
    /** The offset to the particle data.                                */
    int32 PayloadOffset;
    /** The total size of a particle (in bytes).                        */
    int32 ParticleSize;
    /** The stride between particles in the ParticleData array.         */
    int32 ParticleStride;
    /** The number of particles currently active in the emitter.        */
    int32 ActiveParticles;
    /** Monotonically increasing counter. */
    uint32 ParticleCounter;
    /** The maximum number of active particles that can be held in 
     *  the particle data array.
     */
    int32 MaxActiveParticles;
    /** The fraction of time left over from spawning.                   */

    /**
     * Spawn the indicated number of particles.
     *
     * @param Count The number of particles to spawn.
     * @param StartTime			The local emitter time at which to begin spawning particles.
     * @param Increment			The time delta between spawned particles.
     * @param InitialLocation	The initial location of spawned particles.
     * @param InitialVelocity	The initial velocity of spawned particles.
     * @param EventPayload		Event generator payload if events should be triggered.
     */
    void SpawnParticles( int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity, struct FParticleEventInstancePayload* EventPayload );


    void KillParticle(int32 Index);
};
