#pragma once
#include "EnumAsByte.h"
#include "ParticleHelper.h"
#include "HAL/PlatformType.h"
#include "Math/Matrix.h"
#include "EnumAsByte.h"
#include "Math/Transform.h"
#include "Distribution/Distribution.h"
#include "ParticleModuleOrientationAxisLock.h"

class UMaterial;
class UParticleModuleTypeDataMesh;
class UParticleModule;
class UParticleEmitter;
class UParticleLODLevel;
class UParticleSystemComponent;


enum EParticleScreenAlignment
{
    PSA_FacingCameraPosition,
    PSA_Square,
    PSA_Rectangle,
    PSA_Velocity,
    PSA_AwayFromCenter,
    PSA_TypeSpecific,
    PSA_FacingCameraDistanceBlend,
    PSA_MAX,
};


/*-----------------------------------------------------------------------------
    Information compiled from modules to build runtime emitter data.
-----------------------------------------------------------------------------*/

struct FParticleEmitterBuildInfo
{
    /** The required module. */
    class UParticleModuleRequired* RequiredModule;
    /** The spawn module. */
    class UParticleModuleSpawn* SpawnModule;
    /** The spawn-per-unit module. */
    class UParticleModuleSpawnPerUnit* SpawnPerUnitModule;
    /** List of spawn modules that need to be invoked at runtime. */
    TArray<class UParticleModule*> SpawnModules;

    /** The accumulated orbit offset. */
    FDistributionVector OrbitOffset;
    /** The accumulated orbit initial rotation. */
    FDistributionVector OrbitInitialRotation;
    /** The accumulated orbit rotation rate. */
    FDistributionVector OrbitRotationRate;

    /** The color scale of a particle over time. */
    FDistributionVector ColorScale;
    /** The alpha scale of a particle over time. */
    FDistributionFloat AlphaScale;

    /** An additional color scale for allowing parameters to be used for ColorOverLife modules. */
    FDistributionVector DynamicColor;
    /** An additional alpha scale for allowing parameters to be used for ColorOverLife modules. */
    FDistributionFloat DynamicAlpha;

    /** An additional color scale for allowing parameters to be used for ColorScaleOverLife modules. */
    FDistributionVector DynamicColorScale;
    /** An additional alpha scale for allowing parameters to be used for ColorScaleOverLife modules. */
    FDistributionFloat DynamicAlphaScale;

    /** How to scale a particle's size over time. */
    FDistributionVector SizeScale;
    /** The maximum size of a particle. */
    FVector2D MaxSize;
    /** How much to scale a particle's size based on its speed. */
    FVector2D SizeScaleBySpeed;
    /** The maximum amount by which to scale a particle based on its speed. */
    FVector2D MaxSizeScaleBySpeed;

    /** The sub-image index over the particle's life time. */
    FDistributionFloat SubImageIndex;

    /** Drag coefficient. */
    FDistributionFloat DragCoefficient;
    /** Drag scale over life. */
    FDistributionFloat DragScale;

    ///** Enable collision? */
    //bool bEnableCollision;
    ///** How particles respond to collision. */
    //uint8 CollisionResponse;
    //uint8 CollisionMode;
    ///** Radius scale applied to friction. */
    //float CollisionRadiusScale;
    ///** Bias applied to the collision radius. */
    //float CollisionRadiusBias;
    ///** Factor reflection spreading cone when colliding. */
    //float CollisionRandomSpread;
    ///** Random distribution across the reflection spreading cone when colliding. */
    //float CollisionRandomDistribution;
    ///** Friction. */
    //float Friction;
    ///** Collision damping factor. */
    //FComposableFloatDistribution Resilience;
    ///** Collision damping factor scale over life. */
    //FComposableFloatDistribution ResilienceScaleOverLife;

    /** Location of a point source attractor. */
    FVector PointAttractorPosition;
    /** Radius of the point source attractor. */
    float PointAttractorRadius;
    /** Strength of the point attractor. */
    FDistributionFloat PointAttractorStrength;

    /** The per-particle vector field scale. */
    FDistributionFloat VectorFieldScale;
    /** The per-particle vector field scale-over-life. */
    FDistributionFloat VectorFieldScaleOverLife;
    /** Global vector field scale. */
    float GlobalVectorFieldScale;
    /** Global vector field tightness. */
    float GlobalVectorFieldTightness;

    /** Local vector field. */
    class UVectorField* LocalVectorField;
    /** Local vector field transform. */
    FTransform LocalVectorFieldTransform;
    /** Local vector field intensity. */
    float LocalVectorFieldIntensity;
    /** Tightness tweak for local vector fields. */
    float LocalVectorFieldTightness;
    /** Minimum initial rotation applied to local vector fields. */
    FVector LocalVectorFieldMinInitialRotation;
    /** Maximum initial rotation applied to local vector fields. */
    FVector LocalVectorFieldMaxInitialRotation;
    /** Local vector field rotation rate. */
    FVector LocalVectorFieldRotationRate;

    /** Constant acceleration to apply to particles. */
    FVector ConstantAcceleration;

    /** The maximum lifetime of any particle that will spawn. */
    float MaxLifetime;
    /** The maximum rotation rate of particles. */
    float MaxRotationRate;
    /** The estimated maximum number of particles for this emitter. */
    int32 EstimatedMaxActiveParticleCount;

    int32 ScreenAlignment;

    /** An offset in UV space for the positioning of a sprites verticies. */
    FVector2D PivotOffset;

    /** If true, local vector fields ignore the component transform. */
    uint32 bLocalVectorFieldIgnoreComponentTransform : 1;
    /** Tile vector field in x axis? */
    uint32 bLocalVectorFieldTileX : 1;
    /** Tile vector field in y axis? */
    uint32 bLocalVectorFieldTileY : 1;
    /** Tile vector field in z axis? */
    uint32 bLocalVectorFieldTileZ : 1;
    /** Use fix delta time in the simulation? */
    uint32 bLocalVectorFieldUseFixDT : 1;

    /** Particle alignment overrides */
    uint32 bRemoveHMDRoll : 1;
    float MinFacingCameraBlendDistance;
    float MaxFacingCameraBlendDistance;

    /** Default constructor. */
    FParticleEmitterBuildInfo();
};

// Hacky base class to avoid 8 bytes of padding after the vtable
struct FParticleEmitterInstanceFixLayout
{
    virtual ~FParticleEmitterInstanceFixLayout() = default;
};

// struct FLODBurstFired
// {
//     TArray<bool> Fired;
// };

struct FParticleEmitterInstance : FParticleEmitterInstanceFixLayout
{
    UParticleEmitter* SpriteTemplate;

    // Owner
    UParticleSystemComponent* Component;

    int32 CurrentLODLevelIndex;
    UParticleLODLevel* CurrentLODLevel;

    /** If true, halt spawning for this instance.						*/
    uint32 bHaltSpawning : 1;
    /** If true, this emitter has been disabled by game code and some systems to re-enable are not allowed. */
    uint32 bHaltSpawningExternal : 1;

    /** Component can disable Tick and Rendering of this emitter. */
    uint32 bEnabled : 1;
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
    uint32 LoopCount;
    /** The offset to the SubUV payload in the particle data.			*/
    int32 SubUVDataOffset; 

    /** Flag indicating if the render data is dirty.					*/
    int32 IsRenderDataDirty;
    /** true if the emitter has no active particles and will no longer spawn any in the future */
    bool bEmitterIsDone;

    float SpawnFraction;
    /** The number of seconds that have passed since the instance was
     *	created.
     */
    float SecondsSinceCreation;
    /** */
    float EmitterTime;
    /** The current duration fo the emitter instance.					*/
    float EmitterDuration;
    /** The emitter duration at each LOD level for the instance.		*/
    TArray<float> EmitterDurations;
    /** The emitter's delay for the current loop		*/
    float CurrentDelay;
    /** The location of the emitter instance							*/
    FVector Location;
    /** The previous location of the instance.							*/
    FVector OldLocation;
    /** The number of triangles to render								*/
    int32	TrianglesToRender;
    int32 MaxVertexIndex;
    /** The bounding box for the particles.								*/
    // FBoundingBox ParticleBoundingBox;
    /** If true, kill this emitter instance when it is deactivated.		*/
    uint32 bKillOnDeactivate : 1;
    /** if true, kill this emitter instance when it has completed.		*/
    uint32 bKillOnCompleted : 1;

    /** The BurstFire information.										*/
    // TArray<FLODBurstFired> BurstFired;

    // Begin Test
    /** The material to render this instance with.						*/
    UMaterial* CurrentMaterial;
    // End Test

    virtual void SetMeshMaterials(const TArray<UMaterial*>& InMaterials);

    /** If true, the emitter has modules that require loop notification.*/
    uint32 bRequiresLoopNotification : 1;
    /** Whether axis lock is enabled, cached here to avoid finding it from the module each frame */
    uint32 bAxisLockEnabled : 1;
    /** Axis lock flags, cached here to avoid finding it from the module each frame */
    TEnumAsByte<EParticleAxisLock> LockAxisFlags;
    /** The offset to the dynamic parameter payload in the particle data*/
    int32 DynamicParameterDataOffset;
    /** Offset to the light module data payload.						*/
    int32 LightDataOffset;
    float LightVolumetricScatteringIntensity;
    /** The offset to the Camera payload in the particle data.			*/
    int32 CameraPayloadOffset;

    /** The PivotOffset applied to the vertex positions 			*/
    FVector2D PivotOffset;
    /** The offset to the TypeData payload in the particle data.		*/
    int32 TypeDataOffset;
    /** The offset to the TypeData instance payload.					*/
    int32 TypeDataInstanceOffset;

    /** The sort mode to use for this emitter as specified by artist.	*/
    int32 SortMode;

    virtual void Init();

    uint32 RequiredBytes();

    virtual uint32 CalculateParticleStride(uint32 ParticleSize);

    void UpdateTransforms();

    void ResetBurstList();

    FMatrix EmitterToSimulation;
    FMatrix SimulationToWorld;

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
    
    virtual float Tick_EmitterTimeSetup(float delta_time, UParticleLODLevel* lod_level);
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
     * Handle any pre-spawning actions required for particlesk
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

    void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent);

    void SetupEmitterDuration();
    
    void KillParticle(int32 Index);


    /**
     *	Retrieves the dynamic data for the emitter
     */
    virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected)
    {
        return nullptr;
    }

public:
    /**
     * Captures dynamic replay data for this particle system.
     *
     * @param	OutData		[Out] Data will be copied here
     *
     * @return Returns true if successful
     */
    virtual bool FillReplayData( FDynamicEmitterReplayDataBase& OutData ) { return false; }
    /** Get pointer to emitter instance random seed payload data for a particular module */
    FParticleRandomSeedInstancePayload* GetModuleRandomSeedInstanceData(UParticleModule* Module);

    /** Set the HaltSpawning flag */
    virtual void SetHaltSpawning(bool bInHaltSpawning)
    {
        bHaltSpawning = bInHaltSpawning;
    }

    /** Set the bHaltSpawningExternal flag */
    virtual void SetHaltSpawningExternal(bool bInHaltSpawning)
    {
        bHaltSpawningExternal = bInHaltSpawning;
    }

	virtual void GetAllocatedSize(int32& OutNum, int32& OutMax);
    
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
	// TArray<UMaterial*> CurrentMaterials;

	/** Constructor	*/
    FParticleMeshEmitterInstance() = default;

	// virtual void InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent) override;
	virtual void Init() override;
	virtual bool Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount = true) override;
	virtual void Tick(float DeltaTime, bool bSuppressSpawning) override;
	// virtual void UpdateBoundingBox(float DeltaTime) override;
	// virtual uint32 RequiredBytes() override;
	virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime) override;
	virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected) override;
	// virtual bool IsDynamicDataRequired(UParticleLODLevel* CurrentLODLevel) override;

	// virtual void Tick_MaterialOverrides(int32 EmitterIndex) override;

	/**
	 *	Retrieves replay data for the emitter
	 *
	 *	@return	The replay data, or NULL on failure
	 */
	// virtual FDynamicEmitterReplayDataBase* GetReplayData() override;

	/**
	 *	Retrieve the allocated size of this instance.
	 *
	 *	@param	OutNum			The size of this instance
	 *	@param	OutMax			The maximum size of this instance
	 */
	virtual void GetAllocatedSize(int32& OutNum, int32& OutMax) override;

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @param	Mode	Specifies which resource size should be displayed. ( see EResourceSizeMode::Type )
	 * @return  Size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	// virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;

	/**
	 * Returns the offset to the mesh rotation payload, if any.
	 */
	// virtual int32 GetMeshRotationOffset() const override
	// {
	// 	return MeshRotationOffset;
	// }

	/**
	 * Returns true if mesh rotation is active.
	 */
	// virtual bool IsMeshRotationActive() const override
	// {
	// 	return MeshRotationActive;
	// }

	/**
	 * Sets the materials with which mesh particles should be rendered.
	 * @param InMaterials - The materials.
	 */
	virtual void SetMeshMaterials( const TArray<UMaterial*>& InMaterials ) override;

	/**
	 * Gathers material relevance flags for this emitter instance.
	 * @param OutMaterialRelevance - Pointer to where material relevance flags will be stored.
	 * @param LODLevel - The LOD level for which to compute material relevance flags.
	 */
	// virtual void GatherMaterialRelevance(FMaterialRelevance* OutMaterialRelevance, const UParticleLODLevel* LODLevel) const override;

	/**
	 * Gets the materials applied to each section of a mesh.
	 */
	void GetMeshMaterials(TArray<UMaterial*>& OutMaterials, const UParticleLODLevel* LODLevel, bool bLogWarnings = false) const;

protected:

	/**
	 * Captures dynamic replay data for this particle system.
	 *
	 * @param	OutData		[Out] Data will be copied here
	 *
	 * @return Returns true if successful
	 */
	virtual bool FillReplayData( FDynamicEmitterReplayDataBase& OutData ) override;
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
