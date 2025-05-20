#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleSubUV.h"
#include "ParticleSpriteEmitter.h"
#include "TypeData/ParticleModuleTypeDataBase.h"
#include "UObject/ObjectFactory.h"

UParticleEmitter::UParticleEmitter()
    : Super()
    , QualityLevelSpawnRateScale(1.0f)
{
}

bool UParticleEmitter::HasAnyEnabledLODs() const
{
    for (UParticleLODLevel* LodLevel : LODLevels)
    {
        if (LodLevel && LodLevel->bEnabled)
        {
            return true;
        }
    }

    return false;
}

FParticleEmitterInstance* UParticleEmitter::CreateInstance(UParticleSystemComponent* InComponent)
{
    UE_LOG(LogLevel::Error, TEXT("UParticleEmitter::CreateInstance is pure virtual"));
    return NULL;
}


UParticleLODLevel* UParticleEmitter::GetLODLevel(int32 LODLevel)
{
    if (LODLevel >= LODLevels.Num())
    {
        return NULL;
    }

    return LODLevels[LODLevel];
}

void UParticleEmitter::CacheEmitterModuleInfo()
{
	// This assert makes sure that packing is as expected.
	// Added FBaseColor... 
	// Linear color change
	// Added Flags field
    bRequiresLoopNotification = false;
    bAxisLockEnabled = false;
    // bMeshRotationActive = false;
    // LockAxisFlags = EPAL_NONE;
    ModuleOffsetMap.Empty();
    ModuleInstanceOffsetMap.Empty();
    ModuleRandomSeedInstanceOffsetMap.Empty();
    ModulesNeedingInstanceData.Empty();
    ModulesNeedingRandomSeedInstanceData.Empty();
    MeshMaterials.Empty();
    DynamicParameterDataOffset = 0;
    LightDataOffset = 0;
    LightVolumetricScatteringIntensity = 0;
    CameraPayloadOffset = 0;
    ParticleSize = sizeof(FBaseParticle);
    ReqInstanceBytes = 0;
    PivotOffset = FVector2D(-0.5f, -0.5f);
    TypeDataOffset = 0;
    TypeDataInstanceOffset = -1;
    // SubUVAnimation = nullptr;

	UParticleLODLevel* HighLODLevel = GetLODLevel(0);

	UParticleModuleTypeDataBase* HighTypeData = HighLODLevel->TypeDataModule;
	if (HighTypeData)
	{
		int32 ReqBytes = HighTypeData->RequiredBytes(static_cast<UParticleModuleTypeDataBase*>(nullptr));
		if (ReqBytes)
		{
			TypeDataOffset = ParticleSize;
			ParticleSize += ReqBytes;
		}

		int32 TempInstanceBytes = HighTypeData->RequiredBytesPerInstance();
		if (TempInstanceBytes)
		{
			TypeDataInstanceOffset = ReqInstanceBytes;
			ReqInstanceBytes += TempInstanceBytes;
		}
	}

	// Grab required module
	// UParticleModuleRequired* RequiredModule = HighLODLevel->RequiredModule;
	// mesh rotation active if alignment is set
	// bMeshRotationActive = (RequiredModule->ScreenAlignment == PSA_Velocity || RequiredModule->ScreenAlignment == PSA_AwayFromCenter);

	// NOTE: This code assumes that the same module order occurs in all LOD levels

	for (int32 ModuleIdx = 0; ModuleIdx < HighLODLevel->Modules.Num(); ModuleIdx++)
	{
		UParticleModule* ParticleModule = HighLODLevel->Modules[ModuleIdx];

		// Loop notification?
		bRequiresLoopNotification |= (ParticleModule->GetFlag(EModuleFlag::Enabled) && ParticleModule->GetFlag(EModuleFlag::RequiresLoopingNotification));

		if (ParticleModule->IsA(UParticleModuleTypeDataBase::StaticClass()) == false)
		{
			int32 ReqBytes = ParticleModule->RequiredBytes(HighTypeData);
			if (ReqBytes)
			{
				ModuleOffsetMap.Add(ParticleModule, ParticleSize);
				// if (ParticleModule->IsA(UParticleModuleParameterDynamic::StaticClass()) && (DynamicParameterDataOffset == 0))
				// {
				// 	DynamicParameterDataOffset = ParticleSize;
				// }
				// if (ParticleModule->IsA(UParticleModuleLight::StaticClass()) && (LightDataOffset == 0))
				// {
				// 	UParticleModuleLight* ParticleModuleLight = Cast<UParticleModuleLight>(ParticleModule);
				// 	LightVolumetricScatteringIntensity = ParticleModuleLight->VolumetricScatteringIntensity;
				// 	LightDataOffset = ParticleSize;
				// }
				// if (ParticleModule->IsA(UParticleModuleCameraOffset::StaticClass()) && (CameraPayloadOffset == 0))
				// {
				// 	CameraPayloadOffset = ParticleSize;
				// }
				ParticleSize += ReqBytes;
			}

			int32 TempInstanceBytes = ParticleModule->RequiredBytesPerInstance();
			if (TempInstanceBytes > 0)
			{
				// Add the high-lodlevel offset to the lookup map
				ModuleInstanceOffsetMap.Add(ParticleModule, ReqInstanceBytes);
				// Remember that this module has emitter-instance data
				ModulesNeedingInstanceData.Add(ParticleModule);

				// Add all the other LODLevel modules, using the same offset.
				// This removes the need to always also grab the HighestLODLevel pointer.
				for (int32 LODIdx = 1; LODIdx < LODLevels.Num(); LODIdx++)
				{
					UParticleLODLevel* CurLODLevel = LODLevels[LODIdx];
					ModuleInstanceOffsetMap.Add(CurLODLevel->Modules[ModuleIdx], ReqInstanceBytes);
				}
				ReqInstanceBytes += TempInstanceBytes;
			}

			// Add space for per instance random seed value if required
			if ( /** FApp::bUseFixedSeed || */ ParticleModule->GetFlag(EModuleFlag::SupportsRandomSeed))
			{
				// Add the high-lodlevel offset to the lookup map
				ModuleRandomSeedInstanceOffsetMap.Add(ParticleModule, ReqInstanceBytes);
				// Remember that this module has emitter-instance data
				ModulesNeedingRandomSeedInstanceData.Add(ParticleModule);

				// Add all the other LODLevel modules, using the same offset.
				// This removes the need to always also grab the HighestLODLevel pointer.
				for (int32 LODIdx = 1; LODIdx < LODLevels.Num(); LODIdx++)
				{
					UParticleLODLevel* CurLODLevel = LODLevels[LODIdx];
					ModuleRandomSeedInstanceOffsetMap.Add(CurLODLevel->Modules[ModuleIdx], ReqInstanceBytes);
				}

				ReqInstanceBytes += sizeof(FParticleRandomSeedInstancePayload);
			}
		}

		// if (ParticleModule->IsA(UParticleModuleOrientationAxisLock::StaticClass()))
		// {
		// 	UParticleModuleOrientationAxisLock* Module_AxisLock = CastChecked<UParticleModuleOrientationAxisLock>(ParticleModule);
		// 	bAxisLockEnabled = Module_AxisLock->bEnabled;
		// 	LockAxisFlags = Module_AxisLock->LockAxisFlags;
		// }
		// else if (ParticleModule->IsA(UParticleModulePivotOffset::StaticClass()))
		// {
		// 	PivotOffset += Cast<UParticleModulePivotOffset>(ParticleModule)->PivotOffset;
		// }
		// else if (ParticleModule->IsA(UParticleModuleMeshMaterial::StaticClass()))
		// {
		// 	UParticleModuleMeshMaterial* MeshMaterialModule = CastChecked<UParticleModuleMeshMaterial>(ParticleModule);
		// 	if (MeshMaterialModule->bEnabled)
		// 	{
		// 		MeshMaterials = MeshMaterialModule->MeshMaterials;
		// 	}
		// }
		// else
        // if (ParticleModule->IsA(UParticleModuleSubUV::StaticClass()))
		// {
			// USubUVAnimation* ModuleSubUVAnimation = Cast<UParticleModuleSubUV>(ParticleModule)->Animation;
			// SubUVAnimation = ModuleSubUVAnimation && ModuleSubUVAnimation->SubUVTexture && ModuleSubUVAnimation->IsBoundingGeometryValid()
			// 	? ModuleSubUVAnimation
			// 	: NULL;
		// }
		// Perform validation / fixup on some modules that can cause crashes if LODs / Modules are out of sync
		// This should only be applied on uncooked builds to avoid wasting cycles
		// else if ( !FPlatformProperties::RequiresCookedData() )
		// {
		// 	if (ParticleModule->IsA(UParticleModuleLocationBoneSocket::StaticClass()))
		// 	{
		// 		UParticleModuleLocationBoneSocket::ValidateLODLevels(this, ModuleIdx);
		// 	}
		// }

		// Set bMeshRotationActive if module says so
		// if(!bMeshRotationActive && ParticleModule->TouchesMeshRotation())
		// {
		// 	bMeshRotationActive = true;
		// }
	}
}


int32 UParticleEmitter::CreateLODLevel(int32 LODLevel)
{
    int32               LevelIndex		= -1;
	UParticleLODLevel*	CreatedLODLevel	= nullptr;

	if (LODLevels.Num() == 0)
	{
		LODLevel = 0;
	}

	// Is the requested index outside a viable range?
	if ((LODLevel < 0) || (LODLevel > LODLevels.Num()))
	{
		return -1;
	}

	// Create a ParticleLODLevel
	CreatedLODLevel = FObjectFactory::ConstructObject<UParticleLODLevel>(this);
	assert(CreatedLODLevel);

	CreatedLODLevel->Level = LODLevel;
	CreatedLODLevel->bEnabled = true;
	// CreatedLODLevel->ConvertedModules = true;
	CreatedLODLevel->PeakActiveParticles = 0;

	// Determine where to place it...
	if (LODLevels.Num() == 0)
	{
		LODLevels.SetNumZeroed(1);
		LODLevels[0] = CreatedLODLevel;
		CreatedLODLevel->Level	= 0;
	}

	{
		// Create the RequiredModule
		UParticleModuleRequired* RequiredModule = FObjectFactory::ConstructObject<UParticleModuleRequired>(GetOuter());
		assert(RequiredModule);
		// RequiredModule->SetToSensibleDefaults(this);
		CreatedLODLevel->RequiredModule	= RequiredModule;

		// The SpawnRate for the required module
		RequiredModule->bUseLocalSpace			= false;
		RequiredModule->bKillOnDeactivate		= false;
		RequiredModule->bKillOnCompleted		= false;
		RequiredModule->EmitterDuration			= 1.0f;
		RequiredModule->EmitterLoops			= 0;
		// RequiredModule->ParticleBurstMethod		= EPBM_Instant;
// #if WITH_EDITORONLY_DATA
// 		RequiredModule->ModuleEditorColor		= FColor::MakeRandomColor();
// #endif // WITH_EDITORONLY_DATA
		RequiredModule->InterpolationMethod		= PSUVIM_None;
		RequiredModule->SubImages_Horizontal	= 1;
		RequiredModule->SubImages_Vertical		= 1;
		// RequiredModule->bScaleUV				= false;
		RequiredModule->RandomImageTime			= 0.0f;
		RequiredModule->RandomImageChanges		= 0;
		// RequiredModule->bEnabled				= true;

		// RequiredModule->LODValidity = (1 << LODLevel);

        // There must be a spawn module as well...
		UParticleModuleSpawn* SpawnModule = FObjectFactory::ConstructObject<UParticleModuleSpawn>(GetOuter());
		assert(SpawnModule);
		CreatedLODLevel->SpawnModule = SpawnModule;
		// SpawnModule->LODValidity = (1 << LODLevel);
		// UDistributionFloatConstant* ConstantSpawn	= Cast<UDistributionFloatConstant>(SpawnModule->Rate.Distribution);
		// ConstantSpawn->Constant					= 10;
		// ConstantSpawn->bIsDirty					= true;
		// SpawnModule->BurstList.Empty();

		// Copy the TypeData module
		CreatedLODLevel->TypeDataModule			= NULL;
	}

	LevelIndex = CreatedLODLevel->Level;

	// MarkPackageDirty();

	return LevelIndex;
}

UParticleLODLevel* UParticleEmitter::GetCurrentLODLevel(FParticleEmitterInstance* Instance)
{
    return Instance->CurrentLODLevel;
}

FParticleEmitterInstance* UParticleSpriteEmitter::CreateInstance(UParticleSystemComponent* InComponent)
{
    // If this emitter was cooked out or has no valid LOD levels don't create an instance for it.
    if (
        // (bCookedOut == true) ||
        (LODLevels.Num() == 0))
    {
        return NULL;
    }

    FParticleEmitterInstance* Instance = 0;

    UParticleLODLevel* LODLevel	= GetLODLevel(0);
    assert(LODLevel);

    if (LODLevel->TypeDataModule)
    {
        //@todo. This will NOT work for trails/beams!
        Instance = LODLevel->TypeDataModule->CreateInstance(this, InComponent);
    }
    else
    {
        assert(InComponent);
        Instance = new FParticleSpriteEmitterInstance();
        assert(Instance);
        Instance->InitParameters(this, InComponent);
    }

    if (Instance)
    {
        Instance->CurrentLODLevelIndex	= 0;
        Instance->CurrentLODLevel		= LODLevels[Instance->CurrentLODLevelIndex];
        Instance->Init();
    }

    return Instance;
}
