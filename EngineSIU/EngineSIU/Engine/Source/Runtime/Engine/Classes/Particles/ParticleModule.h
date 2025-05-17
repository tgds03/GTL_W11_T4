#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

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
        SupprotsRandomSeed = 1 << 5,
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

    virtual void Update(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime);

    virtual void FinalUpdate(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime);

    bool GetFlag(EModuleFlag::EModuleFlags Flag)
    {
        return (Flags & Flag);
    };

private:
    EModuleFlag::EModuleFlags Flags = EModuleFlag::None;

};
