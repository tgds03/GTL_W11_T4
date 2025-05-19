#pragma once
#include "Container/Array.h"
#include "Math/Color.h"
#include "Math/Vector.h"

#define DECLARE_PARTICLE(Name,Address)		\
	FBaseParticle& Name = *((FBaseParticle*) (Address));

#define DECLARE_PARTICLE_CONST(Name,Address)		\
	const FBaseParticle& Name = *((const FBaseParticle*) (Address));

#define DECLARE_PARTICLE_PTR(Name,Address)		\
	FBaseParticle* Name = (FBaseParticle*) (Address);

#define BEGIN_UPDATE_LOOP																								\
	{																													\
		if((Owner == NULL) || (Owner->Component == NULL)) continue;														\
		int32&			ActiveParticles = Owner->ActiveParticles;														\
		uint32			CurrentOffset	= Offset;																		\
		const uint8*		ParticleData	= Owner->ParticleData;															\
		const uint32		ParticleStride	= Owner->ParticleStride;														\
		uint16*			ParticleIndices	= Owner->ParticleIndices;														\
		for(int32 i=ActiveParticles-1; i>=0; i--)																			\
		{																												\
			const int32	CurrentIndex	= ParticleIndices[i];															\
			const uint8* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;									\
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);												\
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)															\
			{																											\

#define END_UPDATE_LOOP																									\
			}																											\
			CurrentOffset				= Offset;																		\
		}																												\
	}

#define CONTINUE_UPDATE_LOOP																							\
		CurrentOffset = Offset;																							\
		continue;

#define SPAWN_INIT																										\
	if((Owner == NULL) || (Owner->Component == NULL)) continue;															\
	const int32		ActiveParticles	= Owner->ActiveParticles;															\
	const uint32		ParticleStride	= Owner->ParticleStride;															\
	uint32			CurrentOffset	= Offset;																			\
	FBaseParticle&	Particle		= *(ParticleBase);

#define PARTICLE_ELEMENT(Type,Name)																						\
	Type& Name = *((Type*)((uint8*)ParticleBase + CurrentOffset));																\
	CurrentOffset += sizeof(Type);

#define KILL_CURRENT_PARTICLE																							\
	{																													\
		ParticleIndices[i]					= ParticleIndices[ActiveParticles-1];										\
		ParticleIndices[ActiveParticles-1]	= CurrentIndex;																\
		ActiveParticles--;																								\
	}

struct FParticleMeshEmitterInstance;
class UStaticMesh;

struct FBaseParticle
{
    // 16 bytes
    FVector		    OldLocation;			// Last frame's location, used for collision
    float		    RelativeTime;			// Relative time, range is 0 (==spawn) to 1 (==death)

    // 16 bytes
    FVector		    Location;				// Current location
    float		    OneOverMaxLifetime;		// Reciprocal of lifetime
    
    // 16 bytes
    FVector		    BaseVelocity;			// Velocity = BaseVelocity at the start of each frame.
    float		    Rotation;				// Rotation of particle (in Radians)

    // 16 bytes
    FVector		    Velocity;				// Current velocity, gets reset to BaseVelocity each frame to allow 
    float		    BaseRotationRate;		// Initial angular velocity of particle (in Radians per second)

    // 16 bytes
    FVector		    BaseSize;				// Size = BaseSize at the start of each frame
    float		    RotationRate;			// Current rotation rate, gets reset to BaseRotationRate each frame

    // 16 bytes
    FVector		    Size;					// Current size, gets reset to BaseSize each frame
    int32		    Flags;					// Flags indicating various particle states

    // 16 bytes
    FLinearColor	Color;					// Current color of particle.

    // 16 bytes
    FLinearColor	BaseColor;				// Base color of the particle
};

/**
 * Per-particle data sent to the GPU.
 */
struct FParticleSpriteVertex
{
    /** The position of the particle. */
    FVector Position;
    /** The relative time of the particle. */
    float RelativeTime;
    /** The previous position of the particle. */
    FVector	OldPosition;
    /** Value that remains constant over the lifetime of a particle. */
    float ParticleId;
    /** The size of the particle. */
    FVector2D Size;
    /** The rotation of the particle. */
    float Rotation;
    /** The sub-image index for the particle. */
    float SubImageIndex;
    /** The color of the particle. */
    FLinearColor Color;
};

/**
 * Per-particle data sent to the GPU.
 */
struct FParticleSpriteVertexNonInstanced
{
    /** The texture UVs. */
    FVector2D UV;
    /** The position of the particle. */
    FVector Position;
    /** The relative time of the particle. */
    float RelativeTime;
    /** The previous position of the particle. */
    FVector	OldPosition;
    /** Value that remains constant over the lifetime of a particle. */
    float ParticleId;
    /** The size of the particle. */
    FVector2D Size;
    /** The rotation of the particle. */
    float Rotation;
    /** The sub-image index for the particle. */
    float SubImageIndex;
    /** The color of the particle. */
    FLinearColor Color;
};

//	FParticleSpriteVertexDynamicParameter
struct FParticleVertexDynamicParameter
{
    /** The dynamic parameter of the particle			*/
    float			DynamicValue[4];
};

//	FParticleBeamTrailVertex
struct FParticleBeamTrailVertex
{
    /** The position of the particle. */
    FVector Position;
    /** The relative time of the particle. */
    float RelativeTime;
    /** The previous position of the particle. */
    FVector	OldPosition;
    /** Value that remains constant over the lifetime of a particle. */
    float ParticleId;
    /** The size of the particle. */
    FVector2D Size;
    /** The rotation of the particle. */
    float Rotation;
    /** The sub-image index for the particle. */
    float SubImageIndex;
    /** The color of the particle. */
    FLinearColor Color;

    float			Tex_U;
    float			Tex_V;

    /** The second UV set for the particle				*/
    float			Tex_U2;
    float			Tex_V2;
};

// Per-particle data sent to the GPU.
struct FMeshParticleInstanceVertex
{
    /** The color of the particle. */
    FLinearColor Color;

    /** The instance to world transform of the particle. Translation vector is packed into W components. */
    FVector4 Transform[3];

    /** The velocity of the particle, XYZ: direction, W: speed. */
    FVector4 Velocity;

    /** The sub-image texture offsets for the particle. */
    // int16 SubUVParams[4];

    /** The sub-image lerp value for the particle. */
    // float SubUVLerp;

    /** The relative time of the particle. */
    float RelativeTime;
};

/**
 *	General event instance payload.
 */
struct FParticleEventInstancePayload
{
    uint32 bSpawnEventsPresent:1;
    uint32 bDeathEventsPresent:1;
    uint32 bCollisionEventsPresent:1;
    uint32 bBurstEventsPresent:1;

    int32 SpawnTrackingCount;
    int32 DeathTrackingCount;
    int32 CollisionTrackingCount;
    int32 BurstTrackingCount;
};


/*-----------------------------------------------------------------------------
    Particle Sorting Helper
-----------------------------------------------------------------------------*/
struct FParticleOrder
{
    int32 ParticleIndex;

    union
    {
        float Z;
        uint32 C;
    };
	
    FParticleOrder(int32 InParticleIndex,float InZ):
        ParticleIndex(InParticleIndex),
        Z(InZ)
    {}

    FParticleOrder(int32 InParticleIndex,uint32 InC):
        ParticleIndex(InParticleIndex),
        C(InC)
    {}
};

/**
 * Dynamic particle emitter types
 *
 * NOTE: These are serialized out for particle replay data, so be sure to update all appropriate
 *    when changing anything here.
 */
enum EDynamicEmitterType
{
    DET_Unknown = 0,
    DET_Sprite,
    DET_Mesh,
    DET_Beam2,
    DET_Ribbon,
    DET_AnimTrail,
    DET_Custom
};

struct FParticleDataContainer
{
    int32 MemBlockSize;
    int32 ParticleDataNumBytes;
    int32 ParticleIndicesNumShorts;
    uint8* ParticleData; // this is also the memory block we allocated
    uint16* ParticleIndices; // not allocated, this is at the end of the memory block

    FParticleDataContainer()
        : MemBlockSize(0)
        , ParticleDataNumBytes(0)
        , ParticleIndicesNumShorts(0)
        , ParticleData(nullptr)
        , ParticleIndices(nullptr)
    {
    }
    ~FParticleDataContainer()
    {
        Free();
    }
    void Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts) {}
    void Free() {}
};

struct FDynamicEmitterReplayDataBase
{
    /**	The type of emitter. */
    EDynamicEmitterType	eEmitterType;

    /**	The number of particles currently active in this emitter. */
    int32 ActiveParticleCount;

    int32 ParticleStride;
    FParticleDataContainer DataContainer;

    FVector Scale;

    /** Whether this emitter requires sorting as specified by artist.	*/
    int32 SortMode;

    /** MacroUV (override) data **/
    // FMacroUVOverride MacroUVOverride;

    /** Constructor */
    FDynamicEmitterReplayDataBase()
        : eEmitterType( DET_Unknown ),
          ActiveParticleCount( 0 ),
          ParticleStride( 0 ),
          Scale(FVector( 1.0f )),
          SortMode(0)	// Default to PSORTMODE_None		  
    {
    }

    virtual ~FDynamicEmitterReplayDataBase()
    {
    }
};

/** Base class for all emitter types */
struct FDynamicEmitterDataBase
{
    FDynamicEmitterDataBase(const class UParticleModuleRequired* RequiredModule) {}
	
    virtual ~FDynamicEmitterDataBase()
    {
    }

    /** Custom new/delete with recycling */
    // void* operator new(size_t Size);
    // void operator delete(void *RawMemory, size_t Size);
    
    /**
     *	Create the render thread resources for this emitter data
     *
     *	@param	InOwnerProxy	The proxy that owns this dynamic emitter data
     */
    // virtual void UpdateRenderThreadResourcesEmitter(const FParticleSystemSceneProxy* InOwnerProxy)
    // {
    // }

    /**
     *	Release the render thread resources for this emitter data
     *
     *	@param	InOwnerProxy	The proxy that owns this dynamic emitter data
     */
    // virtual void ReleaseRenderThreadResources(const FParticleSystemSceneProxy* InOwnerProxy)
    // {
    // }

    // virtual void GetDynamicMeshElementsEmitter(const FParticleSystemSceneProxy* Proxy, const FSceneView* View, const FSceneViewFamily& ViewFamily, int32 ViewIndex, FMeshElementCollector& Collector) const {}

    /**
     *	Retrieve the material render proxy to use for rendering this emitter. PURE VIRTUAL
     *
     *	@param	bSelected				Whether the object is selected
     *
     *	@return	FMaterialRenderProxy*	The material proxt to render with.
     */
    // virtual const FMaterialRenderProxy* GetMaterialRenderProxy() = 0;

    /** Callback from the renderer to gather simple lights that this proxy wants renderered. */
    // virtual void GatherSimpleLights(const FParticleSystemSceneProxy* Proxy, const FSceneViewFamily& ViewFamily, FSimpleLightArray& OutParticleLights) const {}

    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;

    /** Returns the current macro uv override. Specialized by FGPUSpriteDynamicEmitterData  */
    // virtual const FMacroUVOverride& GetMacroUVOverride() const { return GetSource().MacroUVOverride; }

    /** true if this emitter is currently selected */
    uint32	bSelected:1;
    /** true if this emitter has valid rendering data */
    uint32	bValid:1;

    int32  EmitterIndex;
};

/** Source data base class for Sprite emitters */
struct FDynamicSpriteEmitterReplayDataBase : public FDynamicEmitterReplayDataBase
{
    // UMaterialInterface*				MaterialInterface;
    struct FParticleRequiredModule	*RequiredModule;
    FVector							NormalsSphereCenter;
    FVector							NormalsCylinderDirection;
    float							InvDeltaSeconds;
    FVector						    LWCTile;
    int32							MaxDrawCount;
    int32							OrbitModuleOffset;
    int32							DynamicParameterDataOffset;
    int32							LightDataOffset;
    float							LightVolumetricScatteringIntensity;
    int32							CameraPayloadOffset;
    int32							SubUVDataOffset;
    int32							SubImages_Horizontal;
    int32							SubImages_Vertical;
    bool						bUseLocalSpace;
    bool						bLockAxis;
    uint8						ScreenAlignment;
    uint8						LockAxisFlag;
    uint8						EmitterRenderMode;
    uint8						EmitterNormalsMode;
    FVector2D					PivotOffset;
    bool						bUseVelocityForMotionBlur;
    bool						bRemoveHMDRoll;
    float						MinFacingCameraBlendDistance;
    float						MaxFacingCameraBlendDistance;
	
    /** Constructor */
    FDynamicSpriteEmitterReplayDataBase() {};
    ~FDynamicSpriteEmitterReplayDataBase() {};

    // /** Serialization */
    // virtual void Serialize( FArchive& Ar );

};

struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
    FDynamicSpriteEmitterDataBase(const UParticleModuleRequired* RequiredModule) : 
    FDynamicEmitterDataBase(RequiredModule)
    // , bUsesDynamicParameter( false )
    {
        // MaterialResource = nullptr;
    }

    virtual ~FDynamicSpriteEmitterDataBase()
    {
    }

    /**
     *	Sort the given sprite particles
     *
     *	@param	SorceMode			The sort mode to utilize (EParticleSortMode)
     *	@param	bLocalSpace			true if the emitter is using local space
     *	@param	ParticleCount		The number of particles
     *	@param	ParticleData		The actual particle data
     *	@param	ParticleStride		The stride between entries in the ParticleData array
     *	@param	ParticleIndices		Indirect index list into ParticleData
     *	@param	View				The scene view being rendered
     *	@param	LocalToWorld		The local to world transform of the component rendering the emitter
     *	@param	ParticleOrder		The array to fill in with ordered indices
     */
    // void SortSpriteParticles(int32 SortMode, bool bLocalSpace, 
    //     int32 ParticleCount, const uint8* ParticleData, int32 ParticleStride, const uint16* ParticleIndices,
    //     const FSceneView* View, const FMatrix& LocalToWorld, FParticleOrder* ParticleOrder) const;
    
    /**
     *	Get the vertex stride for the dynamic rendering data
     */
    virtual int32 GetDynamicVertexStride() const = 0;

    /**
     *	Get the vertex stride for the dynamic parameter rendering data
     */
    virtual int32 GetDynamicParameterVertexStride() const = 0;
    
    /**
     *	Get the source replay data for this emitter
     */
    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const = 0;

    /**
 *	Gets the information required for allocating this emitters indices from the global index array.
 */
    virtual void GetIndexAllocInfo(int32& OutNumIndices, int32& OutStride ) const = 0;

    // const FMaterialRenderProxy*	MaterialResource;
};

struct FDynamicSpriteEmitterReplayData	: public FDynamicSpriteEmitterReplayDataBase
{
    
};

/** Dynamic emitter data for sprite emitters */
struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
    FDynamicSpriteEmitterData(const UParticleModuleRequired* RequiredModule) :
    FDynamicSpriteEmitterDataBase(RequiredModule)
    {
    }

    ~FDynamicSpriteEmitterData()
    {
    }

    /** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
    void Init( bool bInSelected );
    
    /**
     *	Get the vertex stride for the dynamic rendering data
     */
    virtual int32 GetDynamicVertexStride() const override
    {
        return sizeof(FParticleSpriteVertex);
    }
    
    /**
     *	Get the vertex stride for the dynamic parameter rendering data
     */
    virtual int32 GetDynamicParameterVertexStride() const override
    {
        return sizeof(FParticleVertexDynamicParameter);
    }
    
    /**
     *	Get the source replay data for this emitter 
     */
    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
    {
        return &Source;
    }
    
    
    /**
     *	Retrieve the vertex and (optional) index required to render this emitter.
     *	Render-thread only
     *
     *	@param	VertexData			The memory to fill the vertex data into
     *	@param	ParticleOrder		The (optional) particle ordering to use
     *	@param	InCameraPosition	The position of the camera in world space.
     *	@param	InLocalToWorld		Transform from local to world space.
     *
     *	@return	bool			true if successful, false if failed
     */
    bool GetVertexAndIndexData(void* VertexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld) const;
    
    
    // /**
    //  *	Retrieve the vertex and (optional) index required to render this emitter.
    //  *  This version for non-instanced platforms.
    //  *	Render-thread only
    //  *
    //  *	@param	VertexData			The memory to fill the vertex data into
    //  *	@param	FillIndexData		The index data to fill in
    //  *	@param	ParticleOrder		The (optional) particle ordering to use
    //  *	@param	InCameraPosition	The position of the camera in world space.
    //  *	@param	InLocalToWorld		Transform from local to world space.
    //  *
    //  *	@return	bool			true if successful, false if failed
    //  */
    // // TODO Deprecated
    // //bool GetVertexAndIndexDataNonInstanced(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, int32 NumVerticesPerParticle) const;
    //
    // /** Gathers simple lights for this emitter. */
    // // virtual void GatherSimpleLights(const FParticleSystemSceneProxy* Proxy, const FSceneViewFamily& ViewFamily, FSimpleLightArray& OutParticleLights) const override;
    // //
    // // virtual void GetDynamicMeshElementsEmitter(const FParticleSystemSceneProxy* Proxy, const FSceneView* View, const FSceneViewFamily& ViewFamily, int32 ViewIndex, FMeshElementCollector& Collector) const override;
    // //
    // // /**
    // //  *	Create the render thread resources for this emitter data
    // //  *
    // //  *	@param	InOwnerProxy	The proxy that owns this dynamic emitter data
    // //  *
    // //  *	@return	bool			true if successful, false if failed
    // //  */
    // // virtual void UpdateRenderThreadResourcesEmitter(const FParticleSystemSceneProxy* InOwnerProxy) override;
    //
    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const override
    {
        return Source;
    }
    
    /** The frame source data for this particle system.  This is everything needed to represent this
        this particle system frame.  It does not include any transient rendering thread data.  Also, for
        non-simulating 'replay' particle systems, this data may have come straight from disk! */
    FDynamicSpriteEmitterReplayData Source;
    
    // /** Uniform parameters. Most fields are filled in when updates are sent to the rendering thread, some are per-view! */
    // // FParticleSpriteUniformParameters UniformParameters;
};

/** Source data for Mesh emitters */
struct FDynamicMeshEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
{
    // int32	SubUVInterpMethod;
    // int32	SubUVDataOffset;
    int32	SubImages_Horizontal;
    int32	SubImages_Vertical;
    // bool	bScaleUV;
    // Mesh Rotation Payload
    // int32	MeshRotationOffset;
    // int32	MeshMotionBlurOffset;
    uint8	MeshAlignment;
    // bool	bMeshRotationActive;
    FVector	LockedAxis;	

    /** Constructor */
    FDynamicMeshEmitterReplayData() : 
        // SubUVInterpMethod( 0 ),
        // SubUVDataOffset( 0 ),
        SubImages_Horizontal( 0 ),
        SubImages_Vertical( 0 ),
        // bScaleUV( false ),
        // MeshRotationOffset( 0 ),
        // MeshMotionBlurOffset( 0 ),
        MeshAlignment( 0 ),
        // bMeshRotationActive( false ),
        LockedAxis(1.0f, 0.0f, 0.0f)
    {
    }
};

struct FDynamicMeshEmitterData : public FDynamicSpriteEmitterDataBase
{
    FDynamicMeshEmitterData(const UParticleModuleRequired* RequiredModule);

    virtual ~FDynamicMeshEmitterData();

    // uint32 GetMeshLODIndexFromProxy(const FParticleSystemSceneProxy *InOwnerProxy) const;
    /** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
    void Init(
        const FParticleMeshEmitterInstance* InEmitterInstance,
        UStaticMesh* InStaticMesh,
        float InLODSizeScale);
    
    /**
     *	Create the render thread resources for this emitter data
     *
     *	@param	InOwnerProxy	The proxy that owns this dynamic emitter data
     *
     *	@return	bool			true if successful, false if failed
     */
    // virtual void UpdateRenderThreadResourcesEmitter(const FParticleSystemSceneProxy* InOwnerProxy) override;

    /**
     *	Release the render thread resources for this emitter data
     *
     *	@param	InOwnerProxy	The proxy that owns this dynamic emitter data
     *
     *	@return	bool			true if successful, false if failed
     */
    // virtual void ReleaseRenderThreadResources(const FParticleSystemSceneProxy* InOwnerProxy) override;

    // virtual void GetDynamicMeshElementsEmitter(const FParticleSystemSceneProxy* Proxy, const FSceneView* View, const FSceneViewFamily& ViewFamily, int32 ViewIndex, FMeshElementCollector& Collector) const override;

    /**
     *	Retrieve the instance data required to render this emitter.
     *	Render-thread only
     *
     *	@param	InstanceData            The memory to fill the vertex data into
     *	@param InLocalToWorld
     */
    void GetInstanceData(void* InstanceData, const FMatrix& InLocalToWorld) const;

    /**
     *	Helper function for retrieving the particle transform.
     *
     *	@param	InParticle					The particle being processed
     *	@param InLocalToWorld
     *	@param	OutTransformMat				The InstanceToWorld transform matrix for the particle
     */
    void GetParticleTransform(const FBaseParticle& InParticle, const FMatrix& InLocalToWorld, FMatrix& OutTransformMat) const;

    // void GetParticlePrevTransform(const FBaseParticle& InParticle, const FParticleSystemSceneProxy* Proxy, const FSceneView* View, FMatrix& OutTransformMat) const;

    void CalculateParticleTransform(
	    const FMatrix& ProxyLocalToWorld,
	    const FVector& ParticleLocation,
		      float    ParticleRotation,
	    const FVector& ParticleVelocity,
	    const FVector& ParticleSize,
	    const FVector& ParticlePayloadInitialOrientation,
	    const FVector& ParticlePayloadRotation,
	    const FVector& ParticlePayloadCameraOffset,
	    const FVector& ParticlePayloadOrbitOffset,
	    // const FVector& ViewOrigin,
	    // const FVector& ViewDirection,
	    FMatrix& OutTransformMat
	    ) const;

    /** Gathers simple lights for this emitter. */
    // virtual void GatherSimpleLights(const FParticleSystemSceneProxy* Proxy, const FSceneViewFamily& ViewFamily, FSimpleLightArray& OutParticleLights) const override;

    /**
     *	Get the vertex stride for the dynamic rendering data
     */
    virtual int32 GetDynamicVertexStride() const override
    {
	    return sizeof(FMeshParticleInstanceVertex);
    }

    virtual int32 GetDynamicParameterVertexStride() const override 
    {
        return 0;
	    // return sizeof(FMeshParticleInstanceVertexDynamicParameter);
    }

    /**
     *	Get the source replay data for this emitter
     */
    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
    {
	    return &Source;
    }

    /**
     *	 Initialize this emitter's vertex factory with the vertex buffers from the mesh's rendering data.
     */
    // void SetupVertexFactory( FRHICommandListBase& RHICmdList, FMeshParticleVertexFactory* InVertexFactory, const FStaticMeshLODResources& LODResources, uint32 LODIdx) const;

    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const override
    {
	    return Source;
    }

    virtual void GetIndexAllocInfo(int32& OutNumIndices, int32& OutStride ) const override {};
    

    /** The frame source data for this particle system.  This is everything needed to represent this
	    this particle system frame.  It does not include any transient rendering thread data.  Also, for
	    non-simulating 'replay' particle systems, this data may have come straight from disk! */
    FDynamicMeshEmitterReplayData Source;

    int32					LastFramePreRendered;

    UStaticMesh*		StaticMesh;
    // TArray<FMaterialRenderProxy*, TInlineAllocator<2>> MeshMaterials;	

    /** offset to FMeshTypeDataPayload */
    uint32 MeshTypeDataOffset;

    // 'orientation' items...
    // These don't need to go into the replay data, as they are constant over the life of the emitter
    /** If true, apply the 'pre-rotation' values to the mesh. */
    uint32 bApplyPreRotation:1;
    /** If true, then use the locked axis setting supplied. Trumps locked axis module and/or TypeSpecific mesh settings. */
    uint32 bUseMeshLockedAxis:1;
    /** If true, then use the camera facing options supplied. Trumps all other settings. */
    uint32 bUseCameraFacing:1;
    /** 
     *	If true, apply 'sprite' particle rotation about the orientation axis (direction mesh is pointing).
     *	If false, apply 'sprite' particle rotation about the camera facing axis.
     */
    uint32 bApplyParticleRotationAsSpin:1;	
    /** 
    *	If true, all camera facing options will point the mesh against the camera's view direction rather than pointing at the cameras location. 
    *	If false, the camera facing will point to the cameras position as normal.
    */
    uint32 bFaceCameraDirectionRatherThanPosition:1;
    /** The EMeshCameraFacingOption setting to use if bUseCameraFacing is true. */
    uint8 CameraFacingOption;

    // bool bUseStaticMeshLODs;
    float LODSizeScale;
    // mutable int32 LastCalculatedMeshLOD;
    const FParticleMeshEmitterInstance* EmitterInstance;
};

// TODO 선택학습

// /** Source data for Beam emitters */
// struct FDynamicBeam2EmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
// {
//     
// };
//
// /** Dynamic emitter data for Beam emitters */
// struct FDynamicBeam2EmitterData : public FDynamicSpriteEmitterDataBase
// {
//     
// };
//
// /** Source data for trail-type emitters */
// struct FDynamicTrailsEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
// {
//     
// };
//
// /** Source data for Ribbon emitters */
// struct FDynamicRibbonEmitterReplayData : public FDynamicTrailsEmitterReplayData
// {
//     
// };
//
// /** Dynamic emitter data for Ribbon emitters */
// struct FDynamicTrailsEmitterData : public FDynamicSpriteEmitterDataBase
// {
//     
// };
//
// /** Dynamic emitter data for Ribbon emitters */
// struct FDynamicRibbonEmitterData : public FDynamicTrailsEmitterData
// {
//     
// };

class FParticleDynamicData
{
public:
    FParticleDynamicData()
        : DynamicEmitterDataArray()
    {
    }

    ~FParticleDynamicData()
    {
        ClearEmitterDataArray();
    }

    /** Custom new/delete with recycling */
    // void* operator new(size_t Size);
    // void operator delete(void *RawMemory, size_t Size);

    void ClearEmitterDataArray()
    {
        for (int32 Index = 0; Index < DynamicEmitterDataArray.Num(); Index++)
        {
            FDynamicEmitterDataBase* Data =	DynamicEmitterDataArray[Index];
            delete Data;
        }
        DynamicEmitterDataArray.Empty();
    }

    // uint32 GetMemoryFootprint( void ) const { return( sizeof( *this ) + DynamicEmitterDataArray.GetAllocatedSize() ); }

    /** The Current Emmitter we are rendering **/
    uint32 EmitterIndex;

    // Variables
    TArray<FDynamicEmitterDataBase*>	DynamicEmitterDataArray;

    /** World space position that UVs generated with the ParticleMacroUV material node will be centered on. */
    FVector SystemPositionForMacroUVs;

    /** World space radius that UVs generated with the ParticleMacroUV material node will tile based on. */
    float SystemRadiusForMacroUVs;

#if WITH_PARTICLE_PERF_STATS
    FParticlePerfStatsContext PerfStatContext;
#endif
};
