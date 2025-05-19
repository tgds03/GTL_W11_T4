#include "ParticleSystemComponent.h"

#include "ParticleEmitterInstances.h"
#include "ParticleLODLevel.h"
#include "ParticleSystem.h"
#include "GameFramework/PlayerController.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"
#include "World/World.h"

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
    TotalActiveParticles = 0;
    this->DeltaTime = DeltaTime;
    ComputeTickComponent();
}


void UParticleSystemComponent::ComputeTickComponent()
{
    // for (int EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); ++EmitterIndex)
    // {
    //     FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];
    //
    //     if (Instance && Instance->SpriteTemplate)
    //     {
    //         UParticleLODLevel* SpriteLODLevel = Instance->GetCurrentLODLevelChecked();
    //         if (SpriteLODLevel /** && SpriteLODLevel.IsEnabled */)
    //         {
    //             Instance->Tick(DeltaTime, false);
    //             TotalActiveParticles += Instance->ActiveParticles;
    //         }
    //     }
    // }
}


// 	FInGameScopedCycleCounter InGameCycleCounter(GetWorld(), EInGamePerfTrackers::VFXSignificance, IsInGameThread() ? EInGamePerfTrackerThreads::GameThread : EInGamePerfTrackerThreads::OtherThread, bIsManagingSignificance);
//
// 	SCOPE_CYCLE_COUNTER(STAT_ParticleComputeTickTime);
// 	FScopeCycleCounterUObject AdditionalScope(AdditionalStatObject(), GET_STATID(STAT_ParticleComputeTickTime));
// 	SCOPE_CYCLE_COUNTER(STAT_ParticlesOverview_GT_CNC);
// 	PARTICLE_PERF_STAT_CYCLES_GT(FParticlePerfStatsContext(GetWorld(), Template, this), TickConcurrent);
//
// 	// Tick Subemitters.
// 	int32 EmitterIndex;
// 	NumSignificantEmitters = 0;
// 	for (EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
// 	{
// 		FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];
// 		FScopeCycleCounterEmitter AdditionalScopeInner(Instance);
// #if WITH_EDITOR
// 		uint32 StartTime = FPlatformTime::Cycles();
// #endif
//
// 		if (EmitterIndex + 1 < EmitterInstances.Num())
// 		{
// 			FParticleEmitterInstance* NextInstance = EmitterInstances[EmitterIndex+1];
// 			FPlatformMisc::Prefetch(NextInstance);
// 		}
//
// 		if (Instance && Instance->SpriteTemplate)
// 		{
// 			check(Instance->SpriteTemplate->LODLevels.Num() > 0);
//
// 			UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
// 			if (SpriteLODLevel && SpriteLODLevel->bEnabled)
// 			{
// 				if (bIsManagingSignificance)
// 				{
// 					bool bEmitterIsSignificant = Instance->SpriteTemplate->IsSignificant(RequiredSignificance);
// 					if (bEmitterIsSignificant)
// 					{
// 						++NumSignificantEmitters;
// 						Instance->SetHaltSpawning(false);
// 						Instance->SetFakeBurstWhenSpawningSupressed(false);
// 						Instance->bEnabled = true;
// 					}
// 					else
// 					{
// 						Instance->SetHaltSpawning(true);
// 						Instance->SetFakeBurstWhenSpawningSupressed(true);
// 						if (Instance->SpriteTemplate->bDisableWhenInsignficant)
// 						{
// 							Instance->bEnabled = false;
// 						}
// 					}
// 				}
// 				else
// 				{
// 					++NumSignificantEmitters;
// 				}
//
// 				Instance->Tick(DeltaTimeTick, bSuppressSpawning);
//
// 				Instance->Tick_MaterialOverrides(EmitterIndex);
// 				TotalActiveParticles += Instance->ActiveParticles;
// 			}
//
// #if WITH_EDITOR
// 			uint32 EndTime = FPlatformTime::Cycles();
// 			Instance->LastTickDurationMs += FPlatformTime::ToMilliseconds(EndTime - StartTime);
// #endif
// 		}
// 	}
// 	if (bAsyncWorkOutstanding)
// 	{
// 		FPlatformMisc::MemoryBarrier();
// 		bAsyncWorkOutstanding = false;
// 	}
//}

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
    //SCOPE_CYCLE_COUNTER(STAT_ParticleSystemComponent_CreateDynamicData);

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

#if WITH_PARTICLE_PERF_STATS
	ParticleDynamicData->PerfStatContext = GetPerfStatsContext();
#endif

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
				// if (SceneProxy)
				{
					++NumMeshEmitterLODIndices;
				}

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
