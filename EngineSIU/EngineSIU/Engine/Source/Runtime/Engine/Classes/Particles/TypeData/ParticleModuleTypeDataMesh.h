#pragma once
#include "ParticleModuleTypeDataBase.h"
#include "Distribution/Distribution.h"
#include "ParticleModuleOrientationAxisLock.h"
#include "ParticleEmitterInstances.h"

enum EMeshCameraFacingOptions : int
{
    XAxisFacing_NoUp,
    XAxisFacing_ZUp,
    XAxisFacing_NegativeZUp,
    XAxisFacing_YUp,
    XAxisFacing_NegativeYUp,

    LockedAxis_ZAxisFacing,
    LockedAxis_NegativeZAxisFacing,
    LockedAxis_YAxisFacing,
    LockedAxis_NegativeYAxisFacing,

    VelocityAligned_ZAxisFacing,
    VelocityAligned_NegativeZAxisFacing,
    VelocityAligned_YAxisFacing,
    VelocityAligned_NegativeYAxisFacing,

    EMeshCameraFacingOptions_MAX,
};

class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
    DECLARE_CLASS(UParticleModuleTypeDataMesh, UParticleModuleTypeDataBase)
public:
    UParticleModuleTypeDataMesh() = default;
    UStaticMesh* Mesh;

    /** Random stream for the initial rotation distribution */
    FRandomStream RandomStream;
    
    /** use the static mesh's LOD setup and switch LODs based on largest particle's screen size*/
    float LODSizeScale;

    /** use the static mesh's LOD setup and switch LODs based on largest particle's screen size*/
    uint8 bUseStaticMeshLODs : 1;

    /** If true, has the meshes cast shadows */
    uint8 CastShadows:1;

    /** UNUSED (the collision module dictates doing collisions) */
    uint8 DoCollisions:1;

    /**
     *	If true, use the emitter material when rendering rather than the one applied
     *	to the static mesh model.
     */
    uint8 bOverrideMaterial : 1;

    /** The 'pre' rotation pitch (in degrees) to apply to the static mesh used. */
    FDistributionVector RollPitchYawRange; //????

    TEnumAsByte<EParticleAxisLock> AxisLockOption; // 이거 다른 곳에서 만들어뒀는데 다시 가져와서 써야 함. 위치가 어디가 가장 적절할지 모르겠음.
    uint8 bCameraFacing : 1;

    virtual bool SupportsSubUV() const override { return true; }
    virtual bool IsAMeshEmitter() const override { return true; }

    virtual void SetToSensibleDefaults() override;
    virtual FParticleEmitterInstance* CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent) override;
    //void CreateDistribution();
};
