#include "ParticleSystemComponent.h"

#include "ParticleEmitterInstances.h"
#include "ParticleLODLevel.h"
#include "ParticleSystem.h"
#include "GameFramework/PlayerController.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"
#include "World/World.h"
#include "RandomStream.h"
#include "ParticleEmitter.h"
#include "ParticleModuleRequired.h"

bool GIsAllowingParticles = true;


UParticleSystemComponent::UParticleSystemComponent()
    : Super(), FXSystem(NULL), Template(nullptr), EmitterInstances()
{
    //Template = nullptr;
    // PrimaryComponentTick.bCanEverTick = true;
    // PrimaryComponentTick.TickGroup = TG_DuringPhysics;
    // PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
    // bTickInEditor = true;
    // MaxTimeBeforeForceUpdateTransform = 5.0f;
    // bAutoActivate = true;
    // bResetOnDetach = false;
    // bOldPositionValid = false;
    // OldPosition = FVector::ZeroVector;

    // RandomStream.Initialize(FApp::bUseFixedSeed ? GetFName() : NAME_None);

    // PartSysVelocity = FVector::ZeroVector;

    WarmupTime = 0.0f;
    // SecondsBeforeInactive = 1.0f;
    // bIsTransformDirty = false;
    // bSkipUpdateDynamicDataDuringTick = false;
    bIsViewRelevanceDirty = true;
//     CustomTimeDilation = 1.0f;
//     bAllowConcurrentTick = true;
//     bAsyncWorkOutstanding = false;
//     PoolingMethod = EPSCPoolMethod::None;
//     bWasActive = false;
// #if WITH_EDITORONLY_DATA
//     EditorDetailMode = -1;
// #endif // WITH_EDITORONLY_DATA
//     SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
//     SetGenerateOverlapEvents(false);
//
//     bCastVolumetricTranslucentShadow = true;
//
//     // Disable receiving decals by default.
//     bReceivesDecals = false;
//
//     // Don't need to call OnUpdateTransform, no physics state to update
//     bWantsOnUpdateTransform = false;
//
//     SavedAutoAttachRelativeScale3D = FVector(1.f, 1.f, 1.f);
//     TimeSinceLastTick = 0;
//
//     RequiredSignificance = EParticleSignificanceLevel::Low;
//     LastSignificantTime = 0.0f;
//     bIsManagingSignificance = 0;
//     bWasManagingSignificance = 0;
//     bIsDuringRegister = 0;
//
//     ManagerHandle = INDEX_NONE;
//     bPendingManagerAdd = false;
//     bPendingManagerRemove = false;
//
//     bExcludeFromLightAttachmentGroup = true;
}


void UParticleSystemComponent::TickComponent(float DeltaTime)
{
    if (bFirstTick)
    {
        bFirstTick = false;
        InitializeSystem();
    }
    TotalActiveParticles = 0;
    this->DeltaTime = DeltaTime;

    bool bRequiresReset = bResetTriggered;
    bResetTriggered = false;
    if (bRequiresReset)
    {
        ForceReset();
    }
    ComputeTickComponent();
}

void UParticleSystemComponent::ForceReset()
{
    if (Template != nullptr)
    {
        Template->UpdateAllModuleLists();
    }
}


void UParticleSystemComponent::ComputeTickComponent()
{
    for (int EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); ++EmitterIndex)
    {
        FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];
    
        if (Instance && Instance->SpriteTemplate)
        {
            UParticleLODLevel* SpriteLODLevel = Instance->GetCurrentLODLevelChecked();
            if (SpriteLODLevel /** && SpriteLODLevel.IsEnabled */)
            {
                Instance->Tick(DeltaTime, false);
                TotalActiveParticles += Instance->ActiveParticles;
            }
        }
    }
}

void UParticleSystemComponent::InitializeSystem()
{
    assert(GetWorld());
    UE_LOG(LogLevel::Display, TEXT("InitializeSystem @ %fs Component=0x%p FXSystem=0x%p"), GetWorld()->TimeSeconds, this, FXSystem);
    
    if (GIsAllowingParticles)
    {
        //if (IsTemplate() == true)
        //{
        //    return;
        //}

        if (Template != nullptr)
        {
            EmitterDelay = Template->Delay;

            if (Template->bUseDelayRange)
            {
                const float	Rand = RandomStream.FRand();
                EmitterDelay = Template->DelayLow + ((Template->Delay - Template->DelayLow) * Rand);
            }
        }

        // Allocate the emitter instances and particle data
        InitParticles();
        AccumTickTime = 0.0;
        // BeginTest
        // bAutoActivate가 들어가야할지 bAutoActive
        if ((IsActive() == false) && (bAutoActive == true) && (bWasDeactivated == false))
        {
            SetActive(true);
        }
    }
}

void UParticleSystemComponent::InitParticles()
{
    assert(GetWorld());

    if (Template != NULL)
    {
        WarmupTime = Template->WarmupTime;
        WarmupTickRate = Template->WarmupTickRate;
        bIsViewRelevanceDirty = true;
        //const int32 GlobalDetailMode = GetCurrentDetailMode();
        const bool bCanEverRender = CanEverRender();

        //simplified version.
        int32 NumInstances = EmitterInstances.Num();
        int32 NumEmitters = Template->Emitters.Num();
        const bool bIsFirstCreate = NumInstances == 0;
        // 임의로 false를 박아뒀음.
        EmitterInstances.SetNumZeroed(NumEmitters, false);

        bWasCompleted = bIsFirstCreate ? false : bWasCompleted;

        bool bClearDynamicData = false;
        int32 PreferredLODLevel = LODLevel;
        bool bSetLodLevels = LODLevel > 0; //We should set the lod level even when creating all emitters if the requested LOD is not 0. 

        for (int32 Idx = 0; Idx < NumEmitters; Idx++)
        {
            UParticleEmitter* Emitter = Template->Emitters[Idx];
            if (Emitter)
            {
                FParticleEmitterInstance* Instance = NumInstances == 0 ? NULL : EmitterInstances[Idx];
                //const bool bDetailModeAllowsRendering = DetailMode <= GlobalDetailMode && (Emitter->DetailModeBitmask & (1 << GlobalDetailMode));
                const bool bShouldCreateAndOrInit = Emitter->HasAnyEnabledLODs() && bCanEverRender;

                if (bShouldCreateAndOrInit)
                {
                    if (Instance)
                    {
                        // TODO CHECk THIs
                        Instance->SetHaltSpawning(false);
                        Instance->SetHaltSpawningExternal(false);
                    }
                    else
                    {
                        Instance = Emitter->CreateInstance(this);
                        EmitterInstances[Idx] = Instance;
                    }

                    if (Instance)
                    {
                        Instance->bEnabled = true;
                        Instance->InitParameters(Emitter, this);
                        Instance->Init();

                        PreferredLODLevel = FMath::Min(PreferredLODLevel, Emitter->LODLevels.Num());
                        bSetLodLevels |= !bIsFirstCreate;//Only set lod levels if we init any instances and it's not the first creation time.
                    }
                }
                else
                {
                    if (Instance)
                    {
#if STATS
                        Instance->PreDestructorCall();
#endif
                        delete Instance;
                        EmitterInstances[Idx] = NULL;
                        bClearDynamicData = true;
                    }
                }
            }
        }

        // if (bClearDynamicData)
        // {
        //     ClearDynamicData();
        // }

        if (bSetLodLevels)
        {
            if (PreferredLODLevel != LODLevel)
            {
                // This should never be higher...
                assert(PreferredLODLevel < LODLevel);
                LODLevel = PreferredLODLevel;
            }

            for (int32 Idx = 0; Idx < EmitterInstances.Num(); Idx++)
            {
                FParticleEmitterInstance* Instance = EmitterInstances[Idx];
                // set the LOD levels here
                if (Instance)
                {
                    Instance->CurrentLODLevelIndex = LODLevel;

                    // small safety net for OR-11322; can be removed if the ensure never fires after the change in SetTemplate (reset all instances LOD indices to 0)
                    if (Instance->CurrentLODLevelIndex >= Instance->SpriteTemplate->LODLevels.Num())
                    {
                        Instance->CurrentLODLevelIndex = Instance->SpriteTemplate->LODLevels.Num() - 1;
                        //assert(false, TEXT("LOD access out of bounds (OR-11322). Please let olaf.piesche or simon.tovey know."));
                    }
                    Instance->CurrentLODLevel = Instance->SpriteTemplate->LODLevels[Instance->CurrentLODLevelIndex];
                }
            }
        }
    }

}

// FDynamicEmitterDataBase* UParticleSystemComponent::CreateDynamicDataFromReplay(FParticleEmitterInstance* EmitterInstance,
//     const FDynamicEmitterReplayDataBase* EmitterReplayData, bool bSelected)
// {
// 	checkSlow(EmitterInstance && EmitterInstance->CurrentLODLevel);
// 	if(EmitterReplayData == nullptr)
// 	{
// 	    return nullptr;
// 	}
//
// 	FScopeCycleCounterEmitter AdditionalScope(EmitterInstance);
// #if WITH_EDITOR
// 	uint32 StartTime = FPlatformTime::Cycles();
// #endif
//
// 	// Allocate the appropriate type of emitter data
// 	FDynamicEmitterDataBase* EmitterData = nullptr;
//
// 	switch( EmitterReplayData->eEmitterType )
// 	{
// 		case DET_Sprite:
// 			{
// 		        UE_LOG(LogLevel::Error, TEXT("Sprite is not Implemented yet"));
// 				// Allocate the dynamic data
// 				// FDynamicSpriteEmitterData* NewEmitterData = new FDynamicSpriteEmitterData(EmitterInstance->CurrentLODLevel->RequiredModule);
// 				//
// 				// // Fill in the source data
// 				// const FDynamicSpriteEmitterReplayData* SpriteEmitterReplayData =
// 				// 	static_cast<const FDynamicSpriteEmitterReplayData*>(EmitterReplayData);
// 				// NewEmitterData->Source = *SpriteEmitterReplayData;
// 				//
// 				// // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
// 				// NewEmitterData->Init( bSelected );
// 				//
// 				// EmitterData = NewEmitterData;
// 			}
// 			break;
//
// 		case DET_Mesh:
// 			{
// 				// Allocate the dynamic data
// 				// PVS-Studio does not understand the checkSlow above, so it is warning us that EmitterInstance->CurrentLODLevel may be null.
// 				FDynamicMeshEmitterData* NewEmitterData = ::new FDynamicMeshEmitterData(EmitterInstance->CurrentLODLevel->RequiredModule); //-V595
//
// 				// Fill in the source data
// 				const FDynamicMeshEmitterReplayData* MeshEmitterReplayData =
// 					static_cast< const FDynamicMeshEmitterReplayData* >( EmitterReplayData );
// 				NewEmitterData->Source = *MeshEmitterReplayData;
//
// 				// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
//
// 				// @todo: Currently we're assuming the original emitter instance is bound to the same mesh as
// 				//        when the replay was generated (safe), and various mesh/material indices are intact.  If
// 				//        we ever support swapping meshes/material on the fly, we'll need cache the mesh
// 				//        reference and mesh component/material indices in the actual replay data.
//
// 				if( EmitterInstance != NULL )
// 				{
// 					FParticleMeshEmitterInstance* MeshEmitterInstance =
// 						static_cast< FParticleMeshEmitterInstance* >( EmitterInstance );
// 					NewEmitterData->Init(
// 						bSelected,
// 						MeshEmitterInstance,
// 						MeshEmitterInstance->MeshTypeData->Mesh,
// 						MeshEmitterInstance->MeshTypeData->bUseStaticMeshLODs,
// 						MeshEmitterInstance->MeshTypeData->LODSizeScale);
// 					EmitterData = NewEmitterData;
// 				}
// 			}
// 			break;
//
// 		case DET_Beam2:
// 			{
// 		        UE_LOG(LogLevel::Error, TEXT("DET_Beam2 is not Implemented yet"));
//
// 				// // Allocate the dynamic data
// 				// FDynamicBeam2EmitterData* NewEmitterData = new FDynamicBeam2EmitterData(EmitterInstance->CurrentLODLevel->RequiredModule);
// 				//
// 				// // Fill in the source data
// 				// const FDynamicBeam2EmitterReplayData* Beam2EmitterReplayData =
// 				// 	static_cast< const FDynamicBeam2EmitterReplayData* >( EmitterReplayData );
// 				// NewEmitterData->Source = *Beam2EmitterReplayData;
// 				//
// 				// // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
// 				// NewEmitterData->Init( bSelected );
// 				//
// 				// EmitterData = NewEmitterData;
// 			}
// 			break;
//
// 		case DET_Ribbon:
// 			{
// 		        UE_LOG(LogLevel::Error, TEXT("DET_Ribbon is not Implemented yet"));
//
// 				// Allocate the dynamic data
// 				// FDynamicRibbonEmitterData* NewEmitterData = new FDynamicRibbonEmitterData(EmitterInstance->CurrentLODLevel->RequiredModule);
// 				//
// 				// // Fill in the source data
// 				// const FDynamicRibbonEmitterReplayData* Trail2EmitterReplayData = static_cast<const FDynamicRibbonEmitterReplayData*>(EmitterReplayData);
// 				// NewEmitterData->Source = *Trail2EmitterReplayData;
// 				// // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
// 				// NewEmitterData->Init(bSelected);
// 				// EmitterData = NewEmitterData;
// 			}
// 			break;
//
// 		case DET_AnimTrail:
// 			{
// 		        UE_LOG(LogLevel::Error, TEXT("AnimTrail is not Implemented yet"));
// 				// Allocate the dynamic data
// 				// FDynamicAnimTrailEmitterData* NewEmitterData = new FDynamicAnimTrailEmitterData(EmitterInstance->CurrentLODLevel->RequiredModule);
// 				// // Fill in the source data
// 				// const FDynamicTrailsEmitterReplayData* AnimTrailEmitterReplayData = static_cast<const FDynamicTrailsEmitterReplayData*>(EmitterReplayData);
// 				// NewEmitterData->Source = *AnimTrailEmitterReplayData;
// 				// // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
// 				// NewEmitterData->Init(bSelected);
// 				// EmitterData = NewEmitterData;
// 			}
// 			break;
//
// 		default:
// 			{
// 				// @todo: Support capture of other particle system types
// 			}
// 			break;
// 	}
// #if STATS
// 	if (EmitterData)
// 	{
// 		EmitterData->StatID = EmitterInstance->SpriteTemplate->GetStatIDRT();
// 	}
// #endif
//
// #if WITH_EDITOR
// 	uint32 EndTime = FPlatformTime::Cycles();
// 	EmitterInstance->LastTickDurationMs += FPlatformTime::ToMilliseconds(EndTime - StartTime);
// #endif
//
// 	return EmitterData;
// }

FParticleDynamicData* UParticleSystemComponent::GetDynamicData()
{
    // QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_CreateDynamicData);

	// FInGameScopedCycleCounter InGameCycleCounter(GetWorld(), EInGamePerfTrackers::VFXSignificance, EInGamePerfTrackerThreads::GameThread, bIsManagingSignificance);

	// Only proceed if we have any live particles or if we're actively replaying/capturing
	if (EmitterInstances.Num() > 0)
	{
		int32 LiveCount = 0;
		for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
		{
			FParticleEmitterInstance* EmitInst = EmitterInstances[EmitterIndex];
			if (EmitInst)
			{
				if (EmitInst->ActiveParticles > 0)
				{
					LiveCount++;
				}
			}
		}

		if (
		    // !bForceLODUpdateFromRenderer &&
			LiveCount == 0
			// && ReplayState == PRS_Disabled
			)
		{
			return nullptr;
		}
	}


	FParticleDynamicData* ParticleDynamicData = new FParticleDynamicData();
	// INC_DWORD_STAT(STAT_DynamicPSysCompCount);
	// INC_DWORD_STAT_BY(STAT_DynamicPSysCompMem, sizeof(FParticleDynamicData));

	if (Template)
	{
		// ParticleDynamicData->SystemPositionForMacroUVs = GetComponentTransform().TransformPosition(Template->MacroUVPosition);
		// ParticleDynamicData->SystemRadiusForMacroUVs = Template->MacroUVRadius;
	}

// #if WITH_PARTICLE_PERF_STATS
// 	ParticleDynamicData->PerfStatContext = GetPerfStatsContext();
// #endif

	// if( ReplayState == PRS_Replaying )
	// {
	// 	SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_CreateDynamicData_Replay);
	// 	// Do we have any replay data to play back?
	// 	UParticleSystemReplay* ReplayData = FindReplayClipForIDNumber( ReplayClipIDNumber );
	// 	if( ReplayData != NULL )
	// 	{
	// 		// Make sure the current frame index is in a valid range
	// 		if( ReplayData->Frames.IsValidIndex( ReplayFrameIndex ) )
	// 		{
	// 			// Grab the current particle system replay frame
	// 			const FParticleSystemReplayFrame& CurReplayFrame = ReplayData->Frames[ ReplayFrameIndex ];
	//
	//
	// 			// Fill the emitter dynamic buffers with data from our replay
	// 			ParticleDynamicData->DynamicEmitterDataArray.Reset();
	// 			ParticleDynamicData->DynamicEmitterDataArray.Reserve(CurReplayFrame.Emitters.Num());
	//
	// 			for (int32 CurEmitterIndex = 0; CurEmitterIndex < CurReplayFrame.Emitters.Num(); ++CurEmitterIndex)
	// 			{
	// 				const FParticleEmitterReplayFrame& CurEmitter = CurReplayFrame.Emitters[ CurEmitterIndex ];
	//
	// 				const FDynamicEmitterReplayDataBase* CurEmitterReplay = CurEmitter.FrameState;
	// 				check( CurEmitterReplay != NULL );
	//
	// 				FParticleEmitterInstance* EmitterInst = NULL;
	// 				if( EmitterInstances.IsValidIndex( CurEmitter.OriginalEmitterIndex ) )
	// 				{
	// 					// Fill dynamic data from the replay frame data for this emitter so we can render it
	// 					// Grab the original emitter instance for that this replay was generated from
	// 					FDynamicEmitterDataBase* NewDynamicEmitterData =
	// 						CreateDynamicDataFromReplay( EmitterInstances[ CurEmitter.OriginalEmitterIndex ], CurEmitterReplay, IsOwnerSelected(), InFeatureLevel );
	//
	//
	// 					if( NewDynamicEmitterData != NULL )
	// 					{
	// 						ParticleDynamicData->DynamicEmitterDataArray.Add(NewDynamicEmitterData);
	// 						NewDynamicEmitterData->EmitterIndex = CurEmitterIndex;
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// }
	// else
	{
		// FParticleSystemReplayFrame* NewReplayFrame = NULL;
		// if( ReplayState == PRS_Capturing )
		// {
		// 	SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_CreateDynamicData_Capture);
		// 	ForceAsyncWorkCompletion(ENSURE_AND_STALL);
		// 	check(IsInGameThread());
		// 	// If we don't have any replay data for this component yet, create some now
		// 	UParticleSystemReplay* ReplayData = FindReplayClipForIDNumber( ReplayClipIDNumber );
		// 	if( ReplayData == NULL )
		// 	{
		// 		// Create a new replay clip!
		// 		ReplayData = NewObject<UParticleSystemReplay>(this);
		//
		// 		// Set the clip ID number
		// 		ReplayData->ClipIDNumber = ReplayClipIDNumber;
		//
		// 		// Add this to the component's list of clips
		// 		ReplayClips.Add( ReplayData );
		//
		// 		// We're modifying the component by adding a new replay clip
		// 		MarkPackageDirty();
		// 	}
		//
		//
		// 	// Add a new frame!
		// 	{
		// 		const int32 NewFrameIndex = ReplayData->Frames.Num();
		// 		new( ReplayData->Frames ) FParticleSystemReplayFrame;
		// 		NewReplayFrame = &ReplayData->Frames[ NewFrameIndex ];
		//
		// 		// We're modifying the component by adding a new frame
		// 		MarkPackageDirty();
		// 	}
		// }

		// Is the particle system allowed to run?
	    // TODO 활성, 비활성화 
		// if( bForcedInActive == false )
		{
			//SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_CreateDynamicData_Gather);
			ParticleDynamicData->DynamicEmitterDataArray.Empty();
			ParticleDynamicData->DynamicEmitterDataArray.Reserve(EmitterInstances.Num());

			int32 NumMeshEmitterLODIndices = 0;

			//QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_GetDynamicData);
			for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
			{
				++NumMeshEmitterLODIndices;

				FDynamicEmitterDataBase* NewDynamicEmitterData = nullptr;
				FParticleEmitterInstance* EmitterInst = EmitterInstances[EmitterIndex];
				if (EmitterInst)
				{
					// FScopeCycleCounterEmitter AdditionalScope(EmitterInst);
#if WITH_EDITOR
					// uint32 StartTime = FPlatformTime::Cycles();
#endif

					// Generate the dynamic data for this emitter
					{
						//SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_GetDynamicData);
						bool bIsOwnerSelected = false;
#if WITH_EDITOR
						// {
						// 	SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_GetDynamicData_Selected);
						// 	bIsOwnerSeleted = IsOwnerSelected();
						// }
#endif
						NewDynamicEmitterData = EmitterInst->GetDynamicData(bIsOwnerSelected);
					}
					if( NewDynamicEmitterData != nullptr)
					{
#if STATS
						// NewDynamicEmitterData->StatID = EmitterInst->SpriteTemplate->GetStatIDRT();
#endif
						NewDynamicEmitterData->bValid = true;
						ParticleDynamicData->DynamicEmitterDataArray.Add( NewDynamicEmitterData );
						NewDynamicEmitterData->EmitterIndex = EmitterIndex;
					    NewDynamicEmitterData->bTranslucent = EmitterInst->CurrentLODLevel->RequiredModule->bIsTranslucent;
						
						// Are we current capturing particle state?
						// if( ReplayState == PRS_Capturing )
						// {
						// 	SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_CreateDynamicData_GatherCapture);
						// 	// Capture replay data for this particle system
						// 	// NOTE: This call should always succeed if GetDynamicData succeeded earlier
						// 	FDynamicEmitterReplayDataBase* NewEmitterReplayData = EmitterInst->GetReplayData();
						// 	check( NewEmitterReplayData != NULL );
						//
						//
						// 	// @todo: We could drastically reduce the size of replays in memory and
						// 	//		on disk by implementing delta compression here.
						//
						// 	// Allocate a new emitter frame
						// 	check(NewReplayFrame != NULL);
						// 	const int32 NewFrameEmitterIndex = NewReplayFrame->Emitters.Num();
						// 	new( NewReplayFrame->Emitters ) FParticleEmitterReplayFrame;
						// 	FParticleEmitterReplayFrame* NewEmitterReplayFrame = &NewReplayFrame->Emitters[ NewFrameEmitterIndex ];
						//
						// 	// Store the replay state for this emitter frame.  Note that this will be
						// 	// deleted when the parent object is garbage collected.
						// 	NewEmitterReplayFrame->EmitterType = NewEmitterReplayData->eEmitterType;
						// 	NewEmitterReplayFrame->OriginalEmitterIndex = EmitterIndex;
						// 	NewEmitterReplayFrame->FrameState = NewEmitterReplayData;
						// }
					}
#if WITH_EDITOR
					// uint32 EndTime = FPlatformTime::Cycles();
					// EmitterInst->LastTickDurationMs += FPlatformTime::ToMilliseconds(EndTime - StartTime);
#endif
				}
			}

			// if (SceneProxy && static_cast<FParticleSystemSceneProxy*>(SceneProxy)->MeshEmitterLODIndices.Num() != NumMeshEmitterLODIndices)
			// {
			// 	ENQUEUE_RENDER_COMMAND(UpdateMeshEmitterLODIndicesCmd)(
			// 		[Proxy = SceneProxy, NumMeshEmitterLODIndices](FRHICommandList&)
			// 	{
			// 		if (Proxy)
			// 		{
			// 			FParticleSystemSceneProxy *ParticleProxy = static_cast<FParticleSystemSceneProxy*>(Proxy);
			// 			ParticleProxy->MeshEmitterLODIndices.Reset();
			// 			ParticleProxy->MeshEmitterLODIndices.AddZeroed(NumMeshEmitterLODIndices);
			// 		}
			// 	});
			// }
		}
	}

	return ParticleDynamicData;
}
