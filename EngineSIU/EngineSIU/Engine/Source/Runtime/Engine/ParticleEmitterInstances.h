#pragma once
#include "ParticleHelper.h"
#include "HAL/PlatformType.h"

class UParticleModuleTypeDataMesh;
class UParticleModule;
class UParticleEmitter;
class UParticleLODLevel;
class UParticleSystemComponent;

// Hacky base class to avoid 8 bytes of padding after the vtable
struct FParticleEmitterInstanceFixLayout
{
    virtual ~FParticleEmitterInstanceFixLayout() = default;
};

struct FParticleEmitterInstance : FParticleEmitterInstanceFixLayout
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
    float EmitterTime;
    float EmitterDuration;

    virtual void ResetParticleParameters(float DeltaTime);

    
    virtual void KillParticles();
    
    /**
     * Ensures enough memory is allocated for the requested number of particles.
     *
     * @param NewMaxActiveParticles		The number of particles for which memory must be allocated.
     * @param bSetMaxActiveCount		If true, update the peak active particles for this LOD.
     * @returns bool					true if memory is allocated for at least NewMaxActiveParticles.
     */
    virtual bool Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount = true);

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

    virtual void Tick_ModuleUpdate(float DeltaTime, UParticleLODLevel* CurrentLODLevel);
    virtual void Tick_ModuleFinalUpdate(float DeltaTime, UParticleLODLevel* CurrentLODLevel);
    
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
	UParticleModuleTypeDataMesh* MeshTypeData;
	bool MeshRotationActive;
	int32 MeshRotationOffset;
	int32 MeshMotionBlurOffset;

	/** The materials to render this instance with.	*/
	TArray<UMaterialInterface*> CurrentMaterials;

	/** Constructor	*/
	FParticleMeshEmitterInstance();

	virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent) override;
	virtual void Init() override;
	virtual bool Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount = true) override;
	virtual void Tick(float DeltaTime, bool bSuppressSpawning) override;
	virtual void UpdateBoundingBox(float DeltaTime) override;
	virtual uint32 RequiredBytes() override;
	virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime) override;
	virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected, ERHIFeatureLevel::Type InFeatureLevel) override;
	virtual bool IsDynamicDataRequired(UParticleLODLevel* CurrentLODLevel) override;

	virtual void Tick_MaterialOverrides(int32 EmitterIndex) override;

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	ENGINE_API virtual FDynamicEmitterReplayDataBase* GetReplayData() override;

	/**
	 *	Retrieve the allocated size of this instance.
	 *
	 *	@param	OutNum			The size of this instance
	 *	@param	OutMax			The maximum size of this instance
	 */
	ENGINE_API virtual void GetAllocatedSize(int32& OutNum, int32& OutMax) override;

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @param	Mode	Specifies which resource size should be displayed. ( see EResourceSizeMode::Type )
	 * @return  Size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	ENGINE_API virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;

	/**
	 * Returns the offset to the mesh rotation payload, if any.
	 */
	virtual int32 GetMeshRotationOffset() const override
	{
		return MeshRotationOffset;
	}

	/**
	 * Returns true if mesh rotation is active.
	 */
	virtual bool IsMeshRotationActive() const override
	{
		return MeshRotationActive;
	}

	/**
	 * Sets the materials with which mesh particles should be rendered.
	 * @param InMaterials - The materials.
	 */
	ENGINE_API virtual void SetMeshMaterials( const TArray<UMaterialInterface*>& InMaterials ) override;

	/**
	 * Gathers material relevance flags for this emitter instance.
	 * @param OutMaterialRelevance - Pointer to where material relevance flags will be stored.
	 * @param LODLevel - The LOD level for which to compute material relevance flags.
	 */
	ENGINE_API virtual void GatherMaterialRelevance(FMaterialRelevance* OutMaterialRelevance, const UParticleLODLevel* LODLevel, ERHIFeatureLevel::Type InFeatureLevel) const override;

	/**
	 * Gets the materials applied to each section of a mesh.
	 */
	ENGINE_API void GetMeshMaterials(TArray<UMaterialInterface*,TInlineAllocator<2> >& OutMaterials, const UParticleLODLevel* LODLevel, ERHIFeatureLevel::Type InFeatureLevel, bool bLogWarnings = false) const;

protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns true if successful
	 */
	ENGINE_API virtual bool FillReplayData( FDynamicEmitterReplayDataBase& OutData ) override;
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
