#include "ParticleSystemComponent.h"

#include "ParticleEmitterInstances.h"
#include "ParticleSystem.h"
#include "GameFramework/PlayerController.h"
#include "World/World.h"

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
// 	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(Effects);
// 	LLM_SCOPE(ELLMTag::Particles);
// 	FInGameScopedCycleCounter InGameCycleCounter(GetWorld(), EInGamePerfTrackers::VFXSignificance, EInGamePerfTrackerThreads::GameThread, bIsManagingSignificance);
// 	SCOPE_CYCLE_COUNTER(STAT_ParticlesOverview_GT);
// 	FScopeCycleCounterUObject AdditionalScope(AdditionalStatObject(), GET_STATID(STAT_ParticlesOverview_GT));
//
// 	if (Template == nullptr || Template->Emitters.Num() == 0)
// 	{
// 		// Disable our tick here, will be enabled when activating
// 		SetComponentTickEnabled(false);
// 		return;
// 	}
//
// 	PARTICLE_PERF_STAT_CYCLES_WITH_COUNT_GT(FParticlePerfStatsContext(GetWorld(), Template, this), TickGameThread, 1);
//
// 	checkf(!IsTickManaged() || !PrimaryComponentTick.IsTickFunctionEnabled(), TEXT("PSC has enabled tick funciton and is also ticking via the tick manager.\nTemplate:%s\nPSC: %s\nParent:%s")
// 	, *Template->GetFullName(), *GetFullName(), GetAttachParent() ? *GetAttachParent()->GetFullName() : TEXT("nullptr"));
//
// 	// control tick rate
// 	// don't tick if enough time hasn't passed
// 	if (TimeSinceLastTick + static_cast<uint32>(DeltaTime*1000.0f) < Template->MinTimeBetweenTicks)
// 	{
// 		TimeSinceLastTick += static_cast<uint32>(DeltaTime*1000.0f);
// 		return;
// 	}
// 	// if enough time has passed, and some of it in previous frames, need to take that into account for DeltaTime
// 	DeltaTime += TimeSinceLastTick / 1000.0f;
// 	TimeSinceLastTick = 0;
//
// 	if (bDeactivateTriggered)
// 	{
// 		DeactivateSystem();
//
// 		if (bWasDeactivated)
// 		{
// 			OnComponentDeactivated.Broadcast(this);
// 		}
// 	}
//
// 	ForceAsyncWorkCompletion(ENSURE_AND_STALL);
// 	SCOPE_CYCLE_COUNTER(STAT_PSysCompTickTime);
//
// 	if (bWasManagingSignificance != bIsManagingSignificance)
// 	{	
// 		bWasManagingSignificance = bIsManagingSignificance;
// 		MarkRenderStateDirty();
// 	}
//
// 	bool bDisallowAsync = false;
//
// 	// Bail out if inactive and not AutoActivate
// 	if ((IsActive() == false) && (bAutoActivate == false))
// 	{
// 		// Disable our tick here, will be enabled when activating
// 		SetComponentTickEnabled(false);
// 		return;
// 	}
// 	DeltaTimeTick = DeltaTime;
//
// 	// Bail out if we are running on a dedicated server and we don't want to update on those
// 	if ((bUpdateOnDedicatedServer == false) && (IsNetMode(NM_DedicatedServer)))
// 	{
// 		if (bAutoDestroy)
// 		{
// 			// We need to destroy the component if the user is expecting us to do it automatically otherwise this component will live forever because HasCompleted() will never get checked
// 			DestroyComponent();
// 		}
// 		else
// 		{
// 			SetComponentTickEnabled(false);
// 		}
// 		return;
// 	}
//
// 	UWorld* World = GetWorld();
// 	check(World);
//
// 	bool bRequiresReset = bResetTriggered;
// 	bResetTriggered = false;
//
// 	// System settings may have been lowered. Support late deactivation.
// 	int32 DetailModeCVar = GetCurrentDetailMode();
// 	const bool bDetailModeAllowsRendering	= DetailMode <= DetailModeCVar;
// 	if (bDetailModeAllowsRendering == false)
// 	{
// 		if (IsActive())
// 		{
// 			DeactivateSystem();
// 			Super::MarkRenderDynamicDataDirty();
// 		}
// 		return;
// 	} 
// 	
// 	// Has the actor position changed to the point where we need to reset the LWC tile
// 	if (RequiresLWCTileRecache(LWCTile, GetComponentLocation()))
// 	{
// 		//-OPT: We may be able to narrow down when a reset is required, like having a GPU emitter, having world space emitters, etc.
// 		//      Cascade generally operates at double precision so it may only be GPU emitters that require a reset.
// 		UE_LOG(LogParticles, Warning, TEXT("PSC(%s - %s) required LWC tile recache and was reset."), *GetFullNameSafe(this), *GetFullNameSafe(Template));
// 		bRequiresReset = true;
// 	}
//
// 	if (bRequiresReset)
// 	{
// 		ForceReset();
// 	}
//
// 	// Bail out if MaxSecondsBeforeInactive > 0 and we haven't been rendered the last MaxSecondsBeforeInactive seconds.
// 	if (bWarmingUp == false)
// 	{
// 		//For now, we're only allowing the SecondsBeforeInactive optimization on looping emitters as it can cause leaks with non-looping effects.
// 		//Longer term, there is likely a better solution.
// 		if (CanSkipTickDueToVisibility())//Cannot skip ticking if we've been deactivated otherwise the system cannot complete correctly.
// 		{
// 			return;
// 		}
//
// 		AccumLODDistanceCheckTime += DeltaTime;
// 		if (AccumLODDistanceCheckTime > Template->LODDistanceCheckTime)
// 		{
// 			SCOPE_CYCLE_COUNTER(STAT_UParticleSystemComponent_LOD);
// 			AccumLODDistanceCheckTime = 0.0f;
//
// 			if (ShouldComputeLODFromGameThread())
// 			{
// 				bool bCalculateLODLevel = 
// 					(bOverrideLODMethod == true) ? (LODMethod == PARTICLESYSTEMLODMETHOD_Automatic) : 
// 						(Template->LODMethod == PARTICLESYSTEMLODMETHOD_Automatic);
// 				if (bCalculateLODLevel == true)
// 				{
// 					FVector EffectPosition = GetComponentLocation();
// 					int32 DesiredLODLevel = DetermineLODLevelForLocation(EffectPosition);
// 					SetLODLevel(DesiredLODLevel);
// 				}
// 			}
// 			else
// 			{
// 				// Periodically force an LOD update from the renderer if we are
// 				// using rendering results to make LOD decisions.
// 				bForceLODUpdateFromRenderer = true;
// 				UpdateLODInformation();
// 			}
// 		}
// 	}
//
// 	bForcedInActive = false;
//
// 	DeltaTime *= CustomTimeDilation;
// 	DeltaTimeTick = DeltaTime;
// 	if (FMath::IsNearlyZero(DeltaTimeTick) && GFXSkipZeroDeltaTime)
// 	{
// 		return;
// 	}
//
// 	AccumTickTime += DeltaTime;
//
// 	// Save player locations
// 	PlayerLocations.Reset();
// 	PlayerLODDistanceFactor.Reset();
//
// #if WITH_EDITOR
// 	// clear tick timers
// 	for (auto Instance : EmitterInstances)
// 	{
// 		if (Instance)
// 		{
// 			Instance->LastTickDurationMs = 0.0f;
// 		}
// 	}
// #endif
//
// 	if (World->IsGameWorld())
// 	{
// 		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
// 		{
// 			APlayerController* PlayerController = Iterator->Get();
// 			if (PlayerController && PlayerController->IsLocalPlayerController())
// 			{
// 				FVector POVLoc;
// 				FRotator POVRotation;
// 				PlayerController->GetPlayerViewPoint(POVLoc, POVRotation);
//
// 				PlayerLocations.Add(POVLoc);
// 				PlayerLODDistanceFactor.Add(PlayerController->LocalPlayerCachedLODDistanceFactor);
// 			}
// 		}
// 	}
//
// 	// Orient the Z axis toward the camera
// 	if (Template->bOrientZAxisTowardCamera)
// 	{
// 		OrientZAxisTowardCamera();
// 	}
//
// 	if (Template->SystemUpdateMode == EPSUM_FixedTime)
// 	{
// 		// Use the fixed delta time!
// 		DeltaTime = Template->UpdateTime_Delta;
// 	}
//
// 	// Clear out the events.
// 	SpawnEvents.Reset();
// 	DeathEvents.Reset();
// 	CollisionEvents.Reset();
// 	BurstEvents.Reset();
// 	TotalActiveParticles = 0;
// 	bNeedsFinalize = true;
// 	
// 	if (!IsTickManaged() || bWarmingUp)
// 	{
// 		if (!ThisTickFunction || !ThisTickFunction->IsCompletionHandleValid() || !CanTickInAnyThread() || FXConsoleVariables::bFreezeParticleSimulation || !FXConsoleVariables::bAllowAsyncTick || !FApp::ShouldUseThreadingForPerformance() ||
// 			GDistributionType == 0) // this may not be absolutely required, however if you are using distributions it will be glacial anyway. If you want to get rid of this, note that some modules use this indirectly as their criteria for CanTickInAnyThread
// 		{
// 			bDisallowAsync = true;
// 		}
//
// 		if (bDisallowAsync)
// 		{
// 			if (!FXConsoleVariables::bFreezeParticleSimulation)
// 			{
// 				ComputeTickComponent_Concurrent();
// 			}
// 			FinalizeTickComponent();
// 		}
// 		else
// 		{
// 			SCOPE_CYCLE_COUNTER(STAT_UParticleSystemComponent_QueueTasks);
//
// 			MarshalParamsForAsyncTick();
// 			{
// 				SCOPE_CYCLE_COUNTER(STAT_UParticleSystemComponent_QueueAsync);
// 				FGraphEventRef OutFinalizeBatchEvent;
// 				FThreadSafeCounter* FinalizeDispatchCounter = nullptr;
// 				FGraphEventArray* Prereqs = FXAsyncBatcher.GetAsyncPrereq(OutFinalizeBatchEvent, FinalizeDispatchCounter);
// 				AsyncWork = TGraphTask<FParticleAsyncTask>::CreateTask(Prereqs, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(this, OutFinalizeBatchEvent, FinalizeDispatchCounter);
// #if !WITH_EDITOR  // we need to not complete until this is done because the game thread finalize task has not beed queued yet
// 				ThisTickFunction->GetCompletionHandle()->DontCompleteUntil(AsyncWork);
// #endif
// 			}
// #if WITH_EDITOR  // we need to queue this here because we need to be able to block and wait on it
// 			{
// 				SCOPE_CYCLE_COUNTER(STAT_UParticleSystemComponent_QueueFinalize);
// 				FGraphEventArray Prereqs;
// 				Prereqs.Add(AsyncWork);
// 				FGraphEventRef Finalize = TGraphTask<FParticleFinalizeTask>::CreateTask(&Prereqs, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(this);
// 				ThisTickFunction->GetCompletionHandle()->DontCompleteUntil(Finalize);
// 			}
// #endif
//
// 			if (CVarFXEarlySchedule.GetValueOnGameThread())
// 			{
// 				PrimaryComponentTick.TickGroup = TG_PrePhysics;
// 				PrimaryComponentTick.EndTickGroup = TG_PostPhysics;
// 			}
// 			else
// 			{
// 				PrimaryComponentTick.TickGroup = TG_DuringPhysics;
// 			}
// 		}
// 	}
}


void UParticleSystemComponent::ComputeTickComponent_Concurrent()
{
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
}
