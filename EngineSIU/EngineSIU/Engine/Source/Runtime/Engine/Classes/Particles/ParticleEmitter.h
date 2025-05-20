#pragma once
#include "UObject/Object.h"

#include "ParticleEmitterInstances.h"
#include "UObject/ObjectMacros.h"

enum EParticleSubUVInterpMethod
{
    PSUVIM_None,
    PSUVIM_Linear,
    PSUVIM_Linear_Blend,
    PSUVIM_Random,
    PSUVIM_Random_Blend,
    PSUVIM_MAX,
};


class UParticleLODLevel;

class UParticleEmitter : public UObject
{
    DECLARE_CLASS(UParticleEmitter, UObject)

public:
    UParticleEmitter() = default;

    TArray<class UParticleLODLevel*> LODLevels;

    /** Map module pointers to their offset into the particle data.		*/
    TMap<UParticleModule*, uint32> ModuleOffsetMap;

    /** Map module pointers to their offset into the instance data.		*/
    TMap<UParticleModule*, uint32> ModuleInstanceOffsetMap;

    /** Map module pointers to their offset into the instance data.		*/
    TMap<UParticleModule*, uint32> ModuleRandomSeedInstanceOffsetMap;

    bool HasAnyEnabledLODs() const;

    // Sets up this emitter with sensible defaults so we can see some particles as soon as its created.
    virtual void SetToSensibleDefaults() {}

    virtual FParticleEmitterInstance* CreateInstance(UParticleSystemComponent* InComponent);

    /** If true, the emitter has modules that require loop notification.*/
    uint32 bRequiresLoopNotification : 1;
    /** Whether axis lock is enabled, cached here to avoid finding it from the module each frame */
    uint32 bAxisLockEnabled : 1;
    /** Axis lock flags, cached here to avoid finding it from the module each frame */
    // TEnumAsByte<EParticleAxisLock> LockAxisFlags;
    /** The offset to the dynamic parameter payload in the particle data*/
    int32 DynamicParameterDataOffset;
    /** Offset to the light module data payload.						*/
    int32 LightDataOffset;
    float LightVolumetricScatteringIntensity;
    /** The offset to the Camera payload in the particle data.			*/
    int32 CameraPayloadOffset;
    /** The total size of a particle (in bytes).						*/
    int32 ParticleSize;
    /** The PivotOffset applied to the vertex positions 			*/
    FVector2D PivotOffset;
    /** The offset to the TypeData payload in the particle data.		*/
    int32 TypeDataOffset;
    /** The offset to the TypeData instance payload.					*/
    int32 TypeDataInstanceOffset;

    int32 ReqInstanceBytes;

    // Array of modules that want emitter instance data
    TArray<UParticleModule*> ModulesNeedingInstanceData;
    // Array of modules that want emitter random seed instance data
    TArray<UParticleModule*> ModulesNeedingRandomSeedInstanceData;

    /** Materials collected from any MeshMaterial modules */
    TArray<UMaterial*> MeshMaterials;

    /** GetCurrentLODLevel
*	Returns the currently set LODLevel. Intended for game-time usage.
*	Assumes that the given LODLevel will be in the [0..# LOD levels] range.
*	
*	@return NULL if the requested LODLevel is not valid.
*			The pointer to the requested UParticleLODLevel if valid.
*/
    UParticleLODLevel* GetCurrentLODLevel(FParticleEmitterInstance* Instance);
    UParticleLODLevel* GetLODLevel(int32 LODLevel);

    virtual void UpdateModuleLists();

    /**
     * Builds data needed for simulation by the emitter from all modules.
     */
    void Build();

    /**
     *	Initial allocation count - overrides calculated peak count if > 0
     */
    int32 InitialAllocationCount;
    float QualityLevelSpawnRateScale;

    void CacheEmitterModuleInfo();
};
