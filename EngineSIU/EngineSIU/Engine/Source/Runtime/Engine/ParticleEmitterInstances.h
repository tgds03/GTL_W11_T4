#pragma once
#include "ParticleHelper.h"
#include "HAL/PlatformType.h"

class UParticleModule;
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
    float SpawnFraction;

    virtual void ResetParticleParameters(float DeltaTime);

    
    virtual void KillParticles();

    virtual void Tick(float DeltaTime, bool bSuppressSpawning);
    
    /**
     *	Tick sub-function that handles spawning of particles
     *
     *	@param	DeltaTime			The current time slice
     *	@param	CurrentLODLevel		The current LOD level for the instance
     *	@param	bSuppressSpawning	true if spawning has been suppressed on the owning particle system component
     *	@param	bFirstTime			true if this is the first time the instance has been ticked
     *
     *	@return	float				The SpawnFraction remaining
     */
    virtual float Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* CurrentLODLevel, bool bSuppressSpawning, bool bFirstTime);
    
    /** Get offset for particle payload data for a particular module */
    uint32 GetModuleDataOffset(UParticleModule* Module);
    /** Get pointer to emitter instance payload data for a particular module */
    uint8* GetModuleInstanceData(UParticleModule* Module);

    /**
     *	Spawn particles for this emitter instance
     *	@param	DeltaTime		The time slice to spawn over
     *	@return	float			The leftover fraction of spawning
     */
    virtual float Spawn(float DeltaTime);
    
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

    
    /**
     * Handle any pre-spawning actions required for particles
     *
     * @param Particle			The particle being spawned.
     * @param InitialLocation	The initial location of the particle.
     * @param InitialVelocity	The initial velocity of the particle.
     */
    virtual void PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity);

    /**
     * Handle any post-spawning actions required by the instance
     *
     * @param	Particle					The particle that was spawned
     * @param	InterpolationPercentage		The percentage of the time slice it was spawned at
     * @param	SpawnTime					The time it was spawned at
     */
    virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime);
    
    void KillParticle(int32 Index);

    
    /**
    * Retrieves the current LOD level and asserts that it is valid.
    */
    class UParticleLODLevel* GetCurrentLODLevelChecked();
};


struct FParticleSpriteEmitterInstance : public FParticleEmitterInstance
{
    
};


struct FParticleMeshEmitterInstance : public FParticleEmitterInstance
{
    
};

// struct FParticleBeam2EmitterInstance : public FParticleEmitterInstance
// {
//     
// };

// struct FParticleTrailsEmitterInstance_Base : public FParticleEmitterInstance
// {
//     
// };

// struct FParticleRibbonEmitterInstance : public FParticleTrailsEmitterInstance_Base
// {
//     
// };
