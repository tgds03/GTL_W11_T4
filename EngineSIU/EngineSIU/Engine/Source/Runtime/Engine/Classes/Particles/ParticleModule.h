#pragma once
#include "EnumAsByte.h"
#include "ParticleHelper.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "RandomStream.h"

struct FBaseParticle;
struct FParticleEmitterInstance;

/** ModuleType
 *	Indicates the kind of emitter the module can be applied to.
 *	ie, EPMT_Beam - only applies to beam emitters.
 *
 *	The TypeData field is present to speed up finding the TypeData module.
 */
enum EModuleType : int
{
    /** General - all emitter types can use it			*/
    EPMT_General,
    /** TypeData - TypeData modules						*/
    EPMT_TypeData,
    /** Beam - only applied to beam emitters			*/
    EPMT_Beam,
    /** Trail - only applied to trail emitters			*/
    EPMT_Trail,
    /** Spawn - all emitter types REQUIRE it			*/
    EPMT_Spawn,
    /** Required - all emitter types REQUIRE it			*/
    EPMT_Required,
    /** Event - event related modules					*/
    EPMT_Event,
    /** Light related modules							*/
    EPMT_Light,
    /** SubUV related modules							*/
    EPMT_SubUV,
    EPMT_MAX,
};

struct FParticleRandomSeedInfo
{
    /** The name to expose to the placed instances for setting this seed */
    FName ParameterName;

    /**
     *	If true, the module will attempt to get the seed from the owner
     *	instance. If that fails, it will fall back to getting it from
     *	the RandomSeeds array.
     */
    uint8 bGetSeedFromInstance : 1;

    /**
     *	If true, the seed value retrieved from the instance will be an
     *	index into the array of seeds.
     */
    uint8 bInstanceSeedIsIndex : 1;

    /**
     *	If true, then reset the seed upon the emitter looping.
     *	For looping environmental effects this should likely be set to false to avoid
     *	a repeating pattern.
     */
    uint8 bResetSeedOnEmitterLooping : 1;

    /**
    *	If true, then randomly select a seed entry from the RandomSeeds array
    */
    uint8 bRandomlySelectSeedArray : 1;

    /**
     *	The random seed values to utilize for the module.
     *	More than 1 means the instance will randomly select one.
     */
    TArray<int32> RandomSeeds;



    FParticleRandomSeedInfo()
        : bGetSeedFromInstance(false)
        , bInstanceSeedIsIndex(false)
        , bResetSeedOnEmitterLooping(true)
        , bRandomlySelectSeedArray(false)
    {}

    FORCEINLINE int32 GetInstancePayloadSize() const
    {
        return ((RandomSeeds.Num() > 0) ? sizeof(FParticleRandomSeedInstancePayload) : 0);
    }

};

namespace EModuleFlag
{
    enum EModuleFlags: uint32
    {
        None = 0,
        Enabled = 1 << 0,
        Editable = 1 << 1,
        SpawnModule = 1 << 2,
        UpdateModule = 1 << 3,
        FinalUpdateModule = 1 << 4,
        SupportsRandomSeed = 1 << 5,
    };
}

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

    /**
     *	Retrieve the ModuleType of this module.
     *
     *	@return	EModuleType		The type of module this is.
     */
    virtual EModuleType	GetModuleType() const	{	return EPMT_General;	}

    FRandomStream& GetRandomStream(FParticleEmitterInstance* Owner);

    /**
     *	Allows the module to prep its 'per-instance' data block.
     *
     *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
     *	@param	InstData	Pointer to the data block for this module.
     */
    virtual uint32	PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData);


    virtual FParticleRandomSeedInfo* GetRandomSeedInfo();

    uint32 PrepRandomSeedInstancePayload(FParticleEmitterInstance* Owner, FParticleRandomSeedInstancePayload* InRandSeedPayload, const FParticleRandomSeedInfo& InRandSeedInfo);
    virtual void Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase);
    virtual void Update(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime);

    virtual void FinalUpdate(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime);

    bool GetFlag(EModuleFlag::EModuleFlags Flag)
    {
        return (Flags & Flag);
    };

protected:
    TEnumAsByte<EModuleFlag::EModuleFlags> Flags = EModuleFlag::None;

};
