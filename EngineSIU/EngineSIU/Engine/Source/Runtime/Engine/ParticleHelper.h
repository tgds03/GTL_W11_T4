#pragma once
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
    int16 SubUVParams[4];

    /** The sub-image lerp value for the particle. */
    float SubUVLerp;

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
    void Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts);
    void Free();
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
    /** Returns the source data for this particle system */
	virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;
    
    int32 EmitterIndex;
};

/** Source data base class for Sprite emitters */
struct FDynamicSpriteEmitterReplayDataBase
    : public FDynamicEmitterReplayDataBase
{
    // UMaterialInterface*				MaterialInterface;
    struct FParticleRequiredModule	*RequiredModule;
    FVector							NormalsSphereCenter;
    FVector							NormalsCylinderDirection;
    float							InvDeltaSeconds;
    FVector						LWCTile;
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
    FDynamicSpriteEmitterReplayDataBase();
    ~FDynamicSpriteEmitterReplayDataBase();

    // /** Serialization */
    // virtual void Serialize( FArchive& Ar );

};

struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
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
     *	Get the source replay data for this emitter
     */
    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const = 0;
};

struct FDynamicSpriteEmitterReplayData	: public FDynamicSpriteEmitterReplayDataBase
{
    
};

/** Dynamic emitter data for sprite emitters */
struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
    /**
 *	Get the vertex stride for the dynamic rendering data
 */
    virtual int32 GetDynamicVertexStride() const override
    {
        return sizeof(FParticleSpriteVertex);
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
     *	@param	FillIndexData		The index data to fill in
     *	@param	ParticleOrder		The (optional) particle ordering to use
     *	@param	InCameraPosition	The position of the camera in world space.
     *	@param	InLocalToWorld		Transform from local to world space.
     *	@param	InstanceFactor		The factor to duplicate instances by.
     *
     *	@return	bool			true if successful, false if failed
     */
    bool GetVertexAndIndexData(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, uint32 InstanceFactor) const;
    

    /**
     *	Retrieve the vertex and (optional) index required to render this emitter.
     *  This version for non-instanced platforms.
     *	Render-thread only
     *
     *	@param	VertexData			The memory to fill the vertex data into
     *	@param	FillIndexData		The index data to fill in
     *	@param	ParticleOrder		The (optional) particle ordering to use
     *	@param	InCameraPosition	The position of the camera in world space.
     *	@param	InLocalToWorld		Transform from local to world space.
     *
     *	@return	bool			true if successful, false if failed
     */
    // TODO Deprecated
    //bool GetVertexAndIndexDataNonInstanced(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, int32 NumVerticesPerParticle) const;
    
    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const override
    {
        return Source;
    }

    /** The frame source data for this particle system.  This is everything needed to represent this
    this particle system frame.  It does not include any transient rendering thread data.  Also, for
    non-simulating 'replay' particle systems, this data may have come straight from disk! */
    FDynamicSpriteEmitterReplayData Source;
};

/** Source data for Mesh emitters */
struct FDynamicMeshEmitterReplayData : public FDynamicSpriteEmitterReplayDataBase
{
    
};

struct FDynamicMeshEmitterData : public FDynamicSpriteEmitterDataBase
{
    /**
     *	Get the vertex stride for the dynamic rendering data
     */
    virtual int32 GetDynamicVertexStride() const override
    {
        return sizeof(FMeshParticleInstanceVertex);
    }
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
