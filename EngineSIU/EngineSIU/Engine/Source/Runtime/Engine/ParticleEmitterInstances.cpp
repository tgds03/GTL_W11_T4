#include "ParticleEmitterInstances.h"

#include "Define.h"
#include "Engine/Engine.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/Event/ParticleModuleEventGenerator.h"

void FParticleEmitterInstance::ResetParticleParameters(float DeltaTime)
{
}

void FParticleEmitterInstance::KillParticles()
{
}


/**
 * Ensures enough memory is allocated for the requested number of particles.
 *
 * @param NewMaxActiveParticles		The number of particles for which memory must be allocated.
 * @param bSetMaxActiveCount		If true, update the peak active particles for this LOD.
 * @returns bool					true if memory is allocated for at least NewMaxActiveParticles.
 */
bool FParticleEmitterInstance::Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticleEmitterInstance_Resize);

	if (GEngine->MaxParticleResize > 0)
	{
		if ((NewMaxActiveParticles < 0) || (NewMaxActiveParticles > GEngine->MaxParticleResize))
		{
			if ((NewMaxActiveParticles < 0) || (NewMaxActiveParticles > GEngine->MaxParticleResizeWarn))
			{
				UE_LOG(LogParticles, Warning, TEXT("Emitter::Resize> Invalid NewMaxActive (%d) for Emitter in PSys %s"),
					NewMaxActiveParticles, 
					Component	? 
								Component->Template ? *(Component->Template->GetPathName()) 
													: *(Component->GetName()) 
								:
								TEXT("INVALID COMPONENT"));
			}

			return false;
		}
	}

	if (NewMaxActiveParticles > MaxActiveParticles)
	{
		// Alloc (or realloc) the data array
		// Allocations > 16 byte are always 16 byte aligned so ParticleData can be used with SSE.
		// NOTE: We don't have to zero the memory here... It gets zeroed when grabbed later.
#if STATS
		{
			// Update the memory stat
			int32 OldMem = (MaxActiveParticles * ParticleStride) + (MaxActiveParticles * sizeof(uint16));
			int32 NewMem = (NewMaxActiveParticles * ParticleStride) + (NewMaxActiveParticles * sizeof(uint16));
			DEC_DWORD_STAT_BY(STAT_GTParticleData, OldMem);
			INC_DWORD_STAT_BY(STAT_GTParticleData, NewMem);
		}
#endif

		{
			ParticleData = (uint8*) FMemory::Realloc(ParticleData, ParticleStride * NewMaxActiveParticles);
			check(ParticleData);

			// Allocate memory for indices.
			if (ParticleIndices == NULL)
			{
				// Make sure that we clear all when it is the first alloc
				MaxActiveParticles = 0;
			}
			ParticleIndices	= (uint16*) FMemory::Realloc(ParticleIndices, sizeof(uint16) * (NewMaxActiveParticles + 1));
		}

		// Fill in default 1:1 mapping.
		for (int32 i=MaxActiveParticles; i<NewMaxActiveParticles; i++)
		{
			ParticleIndices[i] = i;
		}

		// Set the max count
		MaxActiveParticles = NewMaxActiveParticles;
	}

#if STATS
	{
		int32 WastedMem = 
			((MaxActiveParticles * ParticleStride) + (MaxActiveParticles * sizeof(uint16))) - 
			((ActiveParticles * ParticleStride) + (ActiveParticles * sizeof(uint16)));
		INC_DWORD_STAT_BY(STAT_DynamicEmitterGTMem_Waste,WastedMem);
	}
#endif

	// Set the PeakActiveParticles
	if (bSetMaxActiveCount)
	{
		UParticleLODLevel* LODLevel	= SpriteTemplate->GetLODLevel(0);
		check(LODLevel);
		if (MaxActiveParticles > LODLevel->PeakActiveParticles)
		{
			LODLevel->PeakActiveParticles = MaxActiveParticles;
		}
	}

	return true;
}


/**
 *	Tick the instance.
 *
 *	@param	DeltaTime			The time slice to use
 *	@param	bSuppressSpawning	If true, do not spawn during Tick
 */
void FParticleEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
    if (SpriteTemplate == nullptr)
    {
        return;
    }
    if (SpriteTemplate->LODLevels.Num() <= 0)
    {
        return;
    }

    UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    // if (bEnabled)
    {
        KillParticles();
        ResetParticleParameters(DeltaTime);
        Tick_ModuleUpdate(DeltaTime, LODLevel);
        Tick_ModuleFinalUpdate(DeltaTime, LODLevel);
    }

    // If this the FirstTime we are being ticked?
    // bool bFirstTime = (SecondsSinceCreation > 0.0f) ? false : true;
    //
    // // Grab the current LOD level
    // UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    //
    // // Handle EmitterTime setup, looping, etc.
    // float EmitterDelay = Tick_EmitterTimeSetup(DeltaTime, LODLevel);
    //
    // // if (bEnabled)
    // {
    //     // Kill off any dead particles
    //     KillParticles();
    //
    //     // Reset particle parameters.
    //     ResetParticleParameters(DeltaTime);
    //
    //     // Update the particles
    //     CurrentMaterial = LODLevel->RequiredModule->Material;
    //     Tick_ModuleUpdate(DeltaTime, LODLevel);
    //
    //     // Spawn new particles.
    //     SpawnFraction = Tick_SpawnParticles(DeltaTime, LODLevel, bSuppressSpawning, bFirstTime);
    //
    //     // PostUpdate (beams only)
    //     Tick_ModulePostUpdate(DeltaTime, LODLevel);
    //
    //     if (ActiveParticles > 0)
    //     {
    //         // Update the orbit data...
    //         UpdateOrbitData(DeltaTime);
    //         // Calculate bounding box and simulate velocity.
    //         UpdateBoundingBox(DeltaTime);
    //     }
    //
    //     Tick_ModuleFinalUpdate(DeltaTime, LODLevel);
    //
    //     CheckEmitterFinished();
    //
    //     // Invalidate the contents of the vertex/index buffer.
    //     IsRenderDataDirty = 1;
    // }
    // else
    // {
    //     FakeBursts();
    // }
    //
    // // 'Reset' the emitter time so that the delay functions correctly
    // EmitterTime += EmitterDelay;
    //
    // // Store the last delta time.
    // LastDeltaTime = DeltaTime;
    //
    // // Reset particles position offset
    // PositionOffsetThisTick = FVector::ZeroVector;
    //
    // INC_DWORD_STAT_BY(STAT_SpriteParticles, ActiveParticles);
}

float FParticleEmitterInstance::Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* InCurrentLODLevel, bool bSuppressSpawning, bool bFirstTime)
{
    if (!bSuppressSpawning && (EmitterTime >= 0.0f))
    {
        // If emitter is not done - spawn at current rate.
        // If EmitterLoops is 0, then we loop forever, so always spawn.
        if ((InCurrentLODLevel->RequiredModule->EmitterLoops == 0) ||
            (LoopCount < InCurrentLODLevel->RequiredModule->EmitterLoops) ||
            (SecondsSinceCreation < (EmitterDuration * InCurrentLODLevel->RequiredModule->EmitterLoops)) ||
            bFirstTime)
        {
            bFirstTime = false;
            SpawnFraction = Spawn(DeltaTime);
        }
    }
    // else if (bFakeBurstsWhenSpawningSupressed)
    // {
    //     FakeBursts();
    // }
	
    return SpawnFraction;
}

void FParticleEmitterInstance::Tick_ModuleUpdate(float DeltaTime, UParticleLODLevel* CurrentLODLevel)
{
    UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
    for (int ModuleIndex = 0; ModuleIndex < CurrentLODLevel->UpdateModules.Num(); ++ModuleIndex)
    {
        UParticleModule* CurrentModule = CurrentLODLevel->UpdateModules[ModuleIndex];
        if (CurrentModule && CurrentModule->GetFlag(EModuleFlag::Enabled) && CurrentModule->GetFlag(EModuleFlag::UpdateModule))
        {
            CurrentModule->Update(this, GetModuleDataOffset(HighestLODLevel->UpdateModules[ModuleIndex]), DeltaTime);
        }
    }
}

void FParticleEmitterInstance::Tick_ModuleFinalUpdate(float DeltaTime, UParticleLODLevel* CurrentLODLevel)
{
    UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
    for (int ModuleIndex = 0; ModuleIndex < CurrentLODLevel->UpdateModules.Num(); ++ModuleIndex)
    {
        UParticleModule* CurrentModule = CurrentLODLevel->UpdateModules[ModuleIndex];
        if (CurrentModule && CurrentModule->GetFlag(EModuleFlag::Enabled) && CurrentModule->GetFlag(EModuleFlag::UpdateModule))
        {
            CurrentModule->FinalUpdate(this, GetModuleDataOffset(HighestLODLevel->UpdateModules[ModuleIndex]), DeltaTime);
        }
    }
}

uint32 FParticleEmitterInstance::GetModuleDataOffset(UParticleModule* Module)
{
    if (SpriteTemplate == nullptr)
    {
        return 0;
    }

    uint32* Offset = SpriteTemplate->ModuleOffsetMap.Find(Module);
    return (Offset != nullptr) ? *Offset : 0;
}

uint8* FParticleEmitterInstance::GetModuleInstanceData(UParticleModule* Module)
{
    // If there is instance data present, look up the modules offset
    if (InstanceData)
    {
        uint32* Offset = SpriteTemplate->ModuleInstanceOffsetMap.Find(Module);
        if (Offset)
        {
            if(*Offset >= (uint32)InstancePayloadSize)
            {
                return nullptr;
            }
            return &(InstanceData[*Offset]);
        }
    }
    return nullptr;
}


/**
 *	Spawn particles for this emitter instance
 *
 *	@param	DeltaTime		The time slice to spawn over
 *
 *	@return	float			The leftover fraction of spawning
 */
float FParticleEmitterInstance::Spawn(float DeltaTime)
{
	// SCOPE_CYCLE_COUNTER(STAT_SpriteSpawnTime);
	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

	// For beams, we probably want to ignore the SpawnRate distribution,
	// and focus strictly on the BurstList...
	float SpawnRate = 0.0f;
	int32 SpawnCount = 0;
	int32 BurstCount = 0;
	float SpawnRateDivisor = 0.0f;
	float OldLeftover = SpawnFraction;

	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];

	bool bProcessSpawnRate = true;
	bool bProcessBurstList = true;
	// int32 DetailMode = Component->GetCurrentDetailMode();
	//
	// if (SpriteTemplate->QualityLevelSpawnRateScale > 0.0f)
	// {
	// 	// Process all Spawning modules that are present in the emitter.
	// 	for (int32 SpawnModIndex = 0; SpawnModIndex < LODLevel->SpawningModules.Num(); SpawnModIndex++)
	// 	{
	// 		UParticleModuleSpawnBase* SpawnModule = LODLevel->SpawningModules[SpawnModIndex];
	// 		if (SpawnModule && SpawnModule->bEnabled)
	// 		{
	// 			UParticleModule* OffsetModule = HighestLODLevel->SpawningModules[SpawnModIndex];
	// 			uint32 Offset = GetModuleDataOffset(OffsetModule);
	//
	// 			// Update the spawn rate
	// 			int32 Number = 0;
	// 			float Rate = 0.0f;
	// 			if (SpawnModule->GetSpawnAmount(this, Offset, OldLeftover, DeltaTime, Number, Rate) == false)
	// 			{
	// 				bProcessSpawnRate = false;
	// 			}
	//
	// 			Number = FMath::Max<int32>(0, Number);
	// 			Rate = FMath::Max<float>(0.0f, Rate);
	//
	// 			SpawnCount += Number;
	// 			SpawnRate += Rate;
	// 			// Update the burst list
	// 			int32 BurstNumber = 0;
	// 			if (SpawnModule->GetBurstCount(this, Offset, OldLeftover, DeltaTime, BurstNumber) == false)
	// 			{
	// 				bProcessBurstList = false;
	// 			}
	//
	// 			BurstCount += BurstNumber;
	// 		}
	// 	}
	//
	// 	// Figure out spawn rate for this tick.
	// 	if (bProcessSpawnRate)
	// 	{
	// 		float RateScale = LODLevel->SpawnModule->RateScale.GetValue(EmitterTime, Component) * LODLevel->SpawnModule->GetGlobalRateScale();
	// 		SpawnRate += LODLevel->SpawnModule->Rate.GetValue(EmitterTime, Component) * RateScale;
	// 		SpawnRate = FMath::Max<float>(0.0f, SpawnRate);
	// 	}
	//
	// 	// Take Bursts into account as well...
	// 	if (bProcessBurstList)
	// 	{
	// 		int32 Burst = 0;
	// 		float BurstTime = GetCurrentBurstRateOffset(DeltaTime, Burst);
	// 		BurstCount += Burst;
	// 	}
	//
	// 	float QualityMult = SpriteTemplate->GetQualityLevelSpawnRateMult();
	// 	SpawnRate = FMath::Max<float>(0.0f, SpawnRate * QualityMult);
	// 	BurstCount = FMath::CeilToInt(BurstCount * QualityMult);
	// }
	// else
	{
		// Disable any spawning if MediumDetailSpawnRateScale is 0 and we are not in high detail mode
		SpawnRate = 0.0f;
		SpawnCount = 0;
		BurstCount = 0;
	}

	// Spawn new particles...
	// if ((SpawnRate > 0.f) || (BurstCount > 0))
	// {
	// 	float SafetyLeftover = OldLeftover;
	// 	// Ensure continuous spawning... lots of fiddling.
	// 	float	NewLeftover = OldLeftover + DeltaTime * SpawnRate;
	// 	int32		Number		= FMath::FloorToInt(NewLeftover);
	// 	float	Increment	= (SpawnRate > 0.0f) ? (1.f / SpawnRate) : 0.0f;
	// 	float	StartTime = DeltaTime + OldLeftover * Increment - Increment;
	// 	NewLeftover			= NewLeftover - Number;
	//
	// 	// Handle growing arrays.
	// 	bool bProcessSpawn = true;
	// 	int32 NewCount = ActiveParticles + Number + BurstCount;
	//
	// 	if (NewCount > FXConsoleVariables::MaxCPUParticlesPerEmitter)
	// 	{
	// 		int32 MaxNewParticles = FXConsoleVariables::MaxCPUParticlesPerEmitter - ActiveParticles;
	// 		BurstCount = FMath::Min(MaxNewParticles, BurstCount);
	// 		MaxNewParticles -= BurstCount;
	// 		Number = FMath::Min(MaxNewParticles, Number);
	// 		NewCount = ActiveParticles + Number + BurstCount;
	// 	}
	//
	// 	float	BurstIncrement = SpriteTemplate->bUseLegacySpawningBehavior ? (BurstCount > 0.0f) ? (1.f / BurstCount) : 0.0f : 0.0f;
	// 	float	BurstStartTime = SpriteTemplate->bUseLegacySpawningBehavior ? DeltaTime * BurstIncrement : 0.0f;
	//
	// 	if (NewCount >= MaxActiveParticles)
	// 	{
	// 		if (DeltaTime < PeakActiveParticleUpdateDelta)
	// 		{
	// 			bProcessSpawn = Resize(NewCount + FMath::TruncToInt(FMath::Sqrt(FMath::Sqrt((float)NewCount)) + 1));
	// 		}
	// 		else
	// 		{
	// 			bProcessSpawn = Resize((NewCount + FMath::TruncToInt(FMath::Sqrt(FMath::Sqrt((float)NewCount)) + 1)), false);
	// 		}
	// 	}
	//
	// 	if (bProcessSpawn == true)
	// 	{
	// 		FParticleEventInstancePayload* EventPayload = NULL;
	// 		if (LODLevel->EventGenerator)
	// 		{
	// 			EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
	// 			if (EventPayload && !EventPayload->bSpawnEventsPresent && !EventPayload->bBurstEventsPresent)
	// 			{
	// 				EventPayload = NULL;
	// 			}
	// 		}
	//
	// 		const FVector InitialLocation = EmitterToSimulation.GetOrigin();
	//
	// 		// Spawn particles.
	// 		SpawnParticles( Number, StartTime, Increment, InitialLocation, FVector::ZeroVector, EventPayload );
	//
	// 		// Burst particles.
	// 		SpawnParticles(BurstCount, BurstStartTime, BurstIncrement, InitialLocation, FVector::ZeroVector, EventPayload);
	//
	// 		return NewLeftover;
	// 	}
	// 	return SafetyLeftover;
	// }

	return SpawnFraction;
}

/**
 * Spawn the indicated number of particles.
 *
 * @param Count The number of particles to spawn.
 * @param StartTime			The local emitter time at which to begin spawning particles.
 * @param Increment			The time delta between spawned particles.
 * @param InitialLocation	The initial location of spawned particles.
 * @param InitialVelocity	The initial velocity of spawned particles.
 * @param EventPayload		Event generator payload if events should be triggered.
 */
void FParticleEmitterInstance::SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity, struct FParticleEventInstancePayload* EventPayload )
{
	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

	if (ActiveParticles > MaxActiveParticles)
	{
	    return;
	}
	if (LODLevel->EventGenerator == nullptr && EventPayload != nullptr)
	{
	    return;
	}

    Count = FMath::Min(Count, MaxActiveParticles - ActiveParticles);

    auto SpawnInternal = [&]()
    {
        UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
        float SpawnTime = StartTime;
        float Interp = 1.0f;
        const float InterpIncrement = (Count > 0 && Increment > 0.0f) ? (1.0f / static_cast<float>(Count)) : 0.0f;
        for (int32 i = 0; i < Count; ++i)
        {
            if (!(ParticleData && ParticleIndices))
            {
                static bool bErrorReported = false;
                if (!bErrorReported)
                {
                    UE_LOG(LogLevel::Error, TEXT("Detected null particles. Template: %s"), SpriteTemplate->GetName());
                    bErrorReported = true;
                }
                ActiveParticles = 0;
                Count = 0;
                break;
            }
        }
    };

	// // Ensure we don't access particle beyond what is allocated.
	// ensure( ActiveParticles + Count <= MaxActiveParticles );
	// Count = FMath::Min<int32>(Count, MaxActiveParticles - ActiveParticles);
	//
	// if (EventPayload && EventPayload->bBurstEventsPresent && Count > 0)
	// {
	// 	LODLevel->EventGenerator->HandleParticleBurst(this, EventPayload, Count);
	// }
	//
	// auto SpawnInternal = [&](bool bLegacySpawnBehavior)
	// {
	// 	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
	// 	float SpawnTime = StartTime;
	// 	float Interp = 1.0f;
	// 	const float InterpIncrement = (Count > 0 && Increment > 0.0f) ? (1.0f / (float)Count) : 0.0f;
	// 	for (int32 i = 0; i < Count; i++)
	// 	{
	// 		// Workaround to released data.
	// 		if (!ensure(ParticleData && ParticleIndices))
	// 		{
	// 			static bool bErrorReported = false;
	// 			if (!bErrorReported)
	// 			{
	// 			    UE_LOG(LogLevel::Error, TEXT("Detected null particles. Template : %s, Component %s"), *GetNameSafe(Component && Component->IsValidLowLevel() ? Component->Template : nullptr), *GetFullNameSafe(Component));
	// 				bErrorReported = true;
	// 			}
	// 			ActiveParticles = 0;
	// 			Count = 0;
	// 			continue;
	// 		}
	//
	// 		// Workaround to corrupted indices.
	// 		uint16 NextFreeIndex = ParticleIndices[ActiveParticles];
	// 		if (!ensure(NextFreeIndex < MaxActiveParticles))
	// 		{
	// 		    UE_LOG(LogLevel::Error, TEXT("Detected corrupted particle indices. Template : %s, Component %s"), *GetNameSafe(Component && Component->IsValidLowLevel() ? Component->Template : nullptr), *GetFullNameSafe(Component));
	// 			//UE_LOG(LogParticles, Error, TEXT("Detected corrupted particle indices. Template : %s, Component %s"), *GetNameSafe(Component && Component->IsValidLowLevel() ? Component->Template : nullptr), *GetFullNameSafe(Component));
	// 			FixupParticleIndices();
	// 			NextFreeIndex = ParticleIndices[ActiveParticles];
	// 		}
	//
	// 		DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * NextFreeIndex);
	//
	// 		const uint32 CurrentParticleIndex = ActiveParticles++;
	//
	// 		if (bLegacySpawnBehavior)
	// 		{
	// 			StartTime -= Increment;
	// 			Interp -= InterpIncrement;
	// 		}
	//
	// 		PreSpawn(Particle, InitialLocation, InitialVelocity);
	// 		for (int32 ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
	// 		{
	// 			UParticleModule* SpawnModule = LODLevel->SpawnModules[ModuleIndex];
	// 			if (SpawnModule->bEnabled)
	// 			{
	// 				UParticleModule* OffsetModule = HighestLODLevel->SpawnModules[ModuleIndex];
	// 				SpawnModule->Spawn(this, GetModuleDataOffset(OffsetModule), SpawnTime, Particle);
	//
	// 				ensureMsgf(!Particle->Location.ContainsNaN(), TEXT("NaN in Particle Location. Template: %s, Component: %s"), Component ? *GetNameSafe(Component->Template) : TEXT("UNKNOWN"), *GetPathNameSafe(Component));
	// 			}
	// 		}
	// 		PostSpawn(Particle, Interp, SpawnTime);
	//
	// 		// Spawn modules may set a relative time greater than 1.0f to indicate that a particle should not be spawned. We kill these particles.
	// 		if (Particle->RelativeTime > 1.0f)
	// 		{
	// 			KillParticle(CurrentParticleIndex);
	//
	// 			// Process next particle
	// 			continue;
	// 		}
	// 		
	// 		if (EventPayload)
	// 		{
	// 			if (EventPayload->bSpawnEventsPresent)
	// 			{
	// 				LODLevel->EventGenerator->HandleParticleSpawned(this, EventPayload, Particle);
	// 			}
	// 		}
	//
	// 		if (!bLegacySpawnBehavior)
	// 		{
	// 			SpawnTime -= Increment;
	// 			Interp -= InterpIncrement;
	// 		}
	//
	// 		INC_DWORD_STAT(STAT_SpriteParticlesSpawned);
	// 	}
	// };
	//
	// if (SpriteTemplate->bUseLegacySpawningBehavior)
	// {
	// 	SpawnInternal(true);
	// }
	// else
	// {
	// 	SpawnInternal(false);
	// }
}

/**
 * Handle any pre-spawning actions required for particles
 *
 * @param Particle			The particle being spawned.
 * @param InitialLocation	The initial location of the particle.
 * @param InitialVelocity	The initial velocity of the particle.
 */
void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity)
{
    if (Particle == nullptr)
    {
        return;
    }
    // This isn't a problem w/ the FMemory::Memzero call - it's a problem in general!
    if (ParticleSize <= 0)
    {
        return;
    }

    // By default, just clear out the particle
    // FMemory::Memzero(Particle, ParticleSize);
    

    // Initialize the particle location.
    Particle->Location = InitialLocation;
    Particle->BaseVelocity = FVector(InitialVelocity);
    Particle->Velocity = FVector(InitialVelocity);

    // New particles has already updated spawn location
    // Subtract offset here, so deferred location offset in UpdateBoundingBox will return this particle back
    // Particle->Location -= PositionOffsetThisTick;
}


/**
 *	Kill the particle at the given instance
 *
 *	@param	Index		The index of the particle to kill.
 */
void FParticleEmitterInstance::KillParticle(int32 Index)
{
    if (Index < ActiveParticles)
    {
        UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
        FParticleEventInstancePayload* EventPayload = nullptr;
        if (LODLevel->EventGenerator)
        {
            EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
            if (EventPayload && (EventPayload->bDeathEventsPresent == false))
            {
                EventPayload = NULL;
            }
        }

        int32 KillIndex = ParticleIndices[Index];

        // Handle the kill event, if needed
        if (EventPayload)
        {
            const uint8* ParticleBase	= ParticleData + KillIndex * ParticleStride;
            FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);
            // LODLevel->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
        }

        // Move it to the 'back' of the list
        for (int32 i=Index; i < ActiveParticles - 1; i++)
        {
            ParticleIndices[i] = ParticleIndices[i+1];
        }
        ParticleIndices[ActiveParticles-1] = KillIndex;
        ActiveParticles--;

        // INC_DWORD_STAT(STAT_SpriteParticlesKilled);
    }
}

class UParticleLODLevel* FParticleEmitterInstance::GetCurrentLODLevelChecked()
{
    if (SpriteTemplate == nullptr)
    {
        return nullptr;
    }
    
    // UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    // if (LODLevel == nullptr)
    // {
    //     return nullptr;
    // }
    //
    // if (LODLevel->RequiredModule == nullptr)
    // {
    //     return nullptr;
    // }
    // return LODLevel;
}

/**
 *	Handle any post-spawning actions required by the instance
 *
 *	@param	Particle					The particle that was spawned
 *	@param	InterpolationPercentage		The percentage of the time slice it was spawned at
 *	@param	SpawnTime					The time it was spawned at
 */
void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime)
{
    // Interpolate position if using world space.
    UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    // if (LODLevel->RequiredModule->bUseLocalSpace == false)
    // {
    //     if (FVector::DistSquared(OldLocation, Location) > 1.f)
    //     {
    //         Particle->Location += InterpolationPercentage * (OldLocation - Location);	
    //     }
    // }
    //
    // // Offset caused by any velocity
    // Particle->OldLocation = Particle->Location;
    // Particle->Location   += FVector(Particle->Velocity) * SpawnTime;
    //
    // // Store a sequence counter.
    // Particle->Flags |= ((ParticleCounter++) & STATE_CounterMask);
    // Particle->Flags |= STATE_Particle_JustSpawned;
}
