#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleSubUV.h"
#include "TypeData/ParticleModuleTypeDataBase.h"

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
	// MeshMaterials.Empty();
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


UParticleLODLevel* UParticleEmitter::GetCurrentLODLevel(FParticleEmitterInstance* Instance)
{
    return Instance->CurrentLODLevel;
}
