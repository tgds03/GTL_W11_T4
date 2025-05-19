#include "ParticleEmitterInstances.h"

#include "Define.h"
#include "Engine/Engine.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/Event/ParticleModuleEventGenerator.h"
#include "RandomStream.h"
#include "ParticleModuleRequired.h"
#include "Core/HAL/PlatformMemory.h"
#include "Templates/AlignmentTemplates.h"
#include "Runtime/Engine/World/World.h"

#include "ParticleModuleSpawn.h"

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
			ParticleData = (uint8*)FPlatformMemory::Realloc(ParticleData, ParticleStride * NewMaxActiveParticles);
			check(ParticleData);

			// Allocate memory for indices.
			if (ParticleIndices == NULL)
			{
				// Make sure that we clear all when it is the first alloc
				MaxActiveParticles = 0;
			}
			ParticleIndices	= (uint16*)FPlatformMemory::Realloc(ParticleIndices, sizeof(uint16) * (NewMaxActiveParticles + 1));
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
		assert(LODLevel);
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

float FParticleEmitterInstance::Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* CurrentLODLevel, bool bSuppressSpawning, bool bFirstTime)
{
    // if (!bHaltSpawning && !bHaltSpawningExternal && !bSuppressSpawning && (EmitterTime >= 0.0f))
    // {
    //     // If emitter is not done - spawn at current rate.
    //     // If EmitterLoops is 0, then we loop forever, so always spawn.
    //     if ((InCurrentLODLevel->RequiredModule->EmitterLoops == 0) ||
    //         (LoopCount < InCurrentLODLevel->RequiredModule->EmitterLoops) ||
    //         (SecondsSinceCreation < (EmitterDuration * InCurrentLODLevel->RequiredModule->EmitterLoops)) ||
    //         bFirstTime)
    //     {
    //         bFirstTime = false;
    //         SpawnFraction = Spawn(DeltaTime);
    //     }
    // }
    // else if (bFakeBurstsWhenSpawningSupressed)
    // {
    //     FakeBursts();
    // }
	
    return SpawnFraction;
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

FParticleRandomSeedInstancePayload* FParticleEmitterInstance::GetModuleRandomSeedInstanceData(UParticleModule* Module)
{
    // If there is instance data present, look up the modules offset
    if (InstanceData)
    {
        uint32* Offset = SpriteTemplate->ModuleRandomSeedInstanceOffsetMap.Find(Module);
        if (Offset)
        {
            assert(*Offset < (uint32)InstancePayloadSize);
            return (FParticleRandomSeedInstancePayload*)&(InstanceData[*Offset]);
        }
    }
    return NULL;
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

void FParticleEmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent)
{
    SpriteTemplate = InTemplate;
    Component = InComponent;
    SetupEmitterDuration();
}

/**
 *	Calculates the emitter duration for the instance.
 */
void FParticleEmitterInstance::SetupEmitterDuration()
{
    // Validity check
    if (SpriteTemplate == NULL)
    {
        return;
    }

    // Set up the array for each LOD level
    int32 EDCount = EmitterDurations.Num();
    if ((EDCount == 0) || (EDCount != SpriteTemplate->LODLevels.Num()))
    {
        EmitterDurations.Empty();
        // 이후에 바로 채워야 함. uninitialized value가 채워져있음.
        EmitterDurations.InsertUninitialized(0, SpriteTemplate->LODLevels.Num());
    }

    // Calculate the duration for each LOD level
    for (int32 LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
    {
        UParticleLODLevel* TempLOD = SpriteTemplate->LODLevels[LODIndex];
        UParticleModuleRequired* RequiredModule = TempLOD->RequiredModule;

        FRandomStream& RandomStream = RequiredModule->GetRandomStream(this);

        CurrentDelay = RequiredModule->EmitterDelay + Component->EmitterDelay;
        if (RequiredModule->bEmitterDelayUseRange)
        {
            const float	Rand = RandomStream.FRand();
            CurrentDelay = RequiredModule->EmitterDelayLow +
                ((RequiredModule->EmitterDelay - RequiredModule->EmitterDelayLow) * Rand) + Component->EmitterDelay;
        }

        if (RequiredModule->bEmitterDurationUseRange)
        {
            const float	Rand = RandomStream.FRand();
            const float	Duration = RequiredModule->EmitterDurationLow +
                ((RequiredModule->EmitterDuration - RequiredModule->EmitterDurationLow) * Rand);
            EmitterDurations[TempLOD->Level] = Duration + CurrentDelay;
        }
        else
        {
            EmitterDurations[TempLOD->Level] = RequiredModule->EmitterDuration + CurrentDelay;
        }

        if ((LoopCount == 1) && (RequiredModule->bDelayFirstLoopOnly == true) &&
            ((RequiredModule->EmitterLoops == 0) || (RequiredModule->EmitterLoops > 1)))
        {
            EmitterDurations[TempLOD->Level] -= CurrentDelay;
        }
    }

    // Set the current duration
    EmitterDuration = EmitterDurations[CurrentLODLevelIndex];
}

/**
 *	Initialize the instance
 */
void FParticleEmitterInstance::Init()
{
    assert(SpriteTemplate != nullptr);

    // Use highest LOD level for init'ing data, will contain all module types.
    UParticleLODLevel* HighLODLevel = SpriteTemplate->LODLevels[0];

    // Set the current material
    assert(HighLODLevel->RequiredModule);
    CurrentMaterial = HighLODLevel->RequiredModule->Material;

    // If we already have a non-zero ParticleSize, don't need to do most allocation work again
    bool bNeedsInit = (ParticleSize == 0);

    if (bNeedsInit)
    {
        // Copy pre-calculated info
        bRequiresLoopNotification = SpriteTemplate->bRequiresLoopNotification;
        bAxisLockEnabled = SpriteTemplate->bAxisLockEnabled;
        LockAxisFlags = SpriteTemplate->LockAxisFlags;
        DynamicParameterDataOffset = SpriteTemplate->DynamicParameterDataOffset;
        LightDataOffset = SpriteTemplate->LightDataOffset;
        LightVolumetricScatteringIntensity = SpriteTemplate->LightVolumetricScatteringIntensity;
        CameraPayloadOffset = SpriteTemplate->CameraPayloadOffset;
        ParticleSize = SpriteTemplate->ParticleSize;
        PivotOffset = SpriteTemplate->PivotOffset;
        TypeDataOffset = SpriteTemplate->TypeDataOffset;
        TypeDataInstanceOffset = SpriteTemplate->TypeDataInstanceOffset;

        if ((InstanceData == NULL) || (SpriteTemplate->ReqInstanceBytes > InstancePayloadSize))
        {
            //InstanceData = (uint8*)(FPlatformMemory::Realloc(InstanceData, SpriteTemplate->ReqInstanceBytes));
            //InstancePayloadSize = SpriteTemplate->ReqInstanceBytes;
            ///////////////
            void* NewInstanceData = FPlatformMemory::Realloc<EAT_Object>(InstanceData, SpriteTemplate->ReqInstanceBytes, InstancePayloadSize);
            if (NewInstanceData) // Realloc 성공 시에만 업데이트
            {
                InstanceData = static_cast<uint8*>(NewInstanceData);
                InstancePayloadSize = SpriteTemplate->ReqInstanceBytes;
            }
            else if (SpriteTemplate->ReqInstanceBytes > 0)
            {
                // Realloc 실패 처리 (예: 에러 로그, 프로그램 종료)
                // InstanceData는 이전 상태 그대로 유지됨
            }
        }
        FPlatformMemory::Memzero(InstanceData, InstancePayloadSize);

        for (UParticleModule* ParticleModule : SpriteTemplate->ModulesNeedingInstanceData)
        {
            assert(ParticleModule);
            uint8* PrepInstData = GetModuleInstanceData(ParticleModule);
            assert(PrepInstData != nullptr); // Shouldn't be in the list if it doesn't have data
            ParticleModule->PrepPerInstanceBlock(this, (void*)PrepInstData);
        }

        for (UParticleModule* ParticleModule : SpriteTemplate->ModulesNeedingRandomSeedInstanceData)
        {
            assert(ParticleModule);
            FParticleRandomSeedInstancePayload* SeedInstancePayload = GetModuleRandomSeedInstanceData(ParticleModule);
            assert(SeedInstancePayload != nullptr); // Shouldn't be in the list if it doesn't have data
            FParticleRandomSeedInfo* RandomSeedInfo = ParticleModule->GetRandomSeedInfo();
            ParticleModule->PrepRandomSeedInstancePayload(this, SeedInstancePayload, RandomSeedInfo ? *RandomSeedInfo : FParticleRandomSeedInfo());
        }

        // Offset into emitter specific payload (e.g. TrailComponent requires extra bytes).
        PayloadOffset = ParticleSize;

        // Update size with emitter specific size requirements.
        ParticleSize += RequiredBytes();

        // Make sure everything is at least 16 byte aligned so we can use SSE for FVector.
        ParticleSize = Align(ParticleSize, 16);

        // E.g. trail emitters store trailing particles directly after leading one.
        ParticleStride = CalculateParticleStride(ParticleSize);
    }

    // Setup the emitter instance material array...
    SetMeshMaterials(SpriteTemplate->MeshMaterials);

    // Set initial values.
    SpawnFraction = 0;
    SecondsSinceCreation = 0;
    EmitterTime = 0;
    ParticleCounter = 0;

    UpdateTransforms();
    // Begin Test
    //Location = Component->GetComponentLocation();
    Location = Component->GetWorldLocation();
    // End Test
    OldLocation = Location;

    TrianglesToRender = 0;
    MaxVertexIndex = 0;

    if (ParticleData == NULL)
    {
        MaxActiveParticles = 0;
        ActiveParticles = 0;
    }

    ParticleBoundingBox.Init();
    if (HighLODLevel->RequiredModule->RandomImageChanges == 0)
    {
        HighLODLevel->RequiredModule->RandomImageTime = 1.0f;
    }
    else
    {
        HighLODLevel->RequiredModule->RandomImageTime = 0.99f / (HighLODLevel->RequiredModule->RandomImageChanges + 1);
    }

    // Resize to sensible default.
    //if (bNeedsInit &&
    //    Component->GetWorld()->IsGameWorld() == true &&
    //    // Only presize if any particles will be spawned 
    //    SpriteTemplate->QualityLevelSpawnRateScale > 0)
    if (bNeedsInit && SpriteTemplate->QualityLevelSpawnRateScale > 0)
    {
        if ((HighLODLevel->PeakActiveParticles > 0) || (SpriteTemplate->InitialAllocationCount > 0))
        {
            // In-game... we assume the editor has set this properly, but still clamp at 100 to avoid wasting
            // memory.
            if (SpriteTemplate->InitialAllocationCount > 0)
            {
                Resize(FMath::Min(SpriteTemplate->InitialAllocationCount, 100));
            }
            else
            {
                Resize(FMath::Min(HighLODLevel->PeakActiveParticles, 100));
            }
        }
        else
        {
            // This is to force the editor to 'select' a value
            Resize(10);
        }
    }

    LoopCount = 0;

    if (bNeedsInit)
    {
        //QUICK_SCOPE_CYCLE_COUNTER(STAT_AllocateBurstLists);
    // Propagate killon flags
        bKillOnDeactivate = HighLODLevel->RequiredModule->bKillOnDeactivate;
        bKillOnCompleted = HighLODLevel->RequiredModule->bKillOnCompleted;

        // Propagate sorting flag.
        SortMode = HighLODLevel->RequiredModule->SortMode;

        // Reset the burst lists
        if (BurstFired.Num() < SpriteTemplate->LODLevels.Num())
        {
            BurstFired.AddZeroed(SpriteTemplate->LODLevels.Num() - BurstFired.Num());
        }

        for (int32 LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
        {
            UParticleLODLevel* LODLevel = SpriteTemplate->LODLevels[LODIndex];
            assert(LODLevel);
            FLODBurstFired& LocalBurstFired = BurstFired[LODIndex];
            if (LocalBurstFired.Fired.Num() < LODLevel->SpawnModule->BurstList.Num())
            {
                LocalBurstFired.Fired.AddZeroed(LODLevel->SpawnModule->BurstList.Num() - LocalBurstFired.Fired.Num());
            }
        }
    }

    ResetBurstList();

#if WITH_EDITORONLY_DATA
    //Check for SubUV module to see if it has SubUVAnimation to move data to required module
    for (auto CurrModule : HighLODLevel->Modules)
    {
        if (CurrModule->IsA(UParticleModuleSubUV::StaticClass()))
        {
            UParticleModuleSubUV* SubUVModule = (UParticleModuleSubUV*)CurrModule;

            if (SubUVModule->Animation)
            {
                HighLODLevel->RequiredModule->AlphaThreshold = SubUVModule->Animation->AlphaThreshold;
                HighLODLevel->RequiredModule->BoundingMode = SubUVModule->Animation->BoundingMode;
                HighLODLevel->RequiredModule->OpacitySourceMode = SubUVModule->Animation->OpacitySourceMode;
                HighLODLevel->RequiredModule->CutoutTexture = SubUVModule->Animation->SubUVTexture;

                SubUVModule->Animation = nullptr;

                HighLODLevel->RequiredModule->CacheDerivedData();
                HighLODLevel->RequiredModule->InitBoundingGeometryBuffer();
            }
        }
    }
#endif //WITH_EDITORONLY_DATA

    // Tag it as dirty w.r.t. the renderer
    IsRenderDataDirty = 1;

    bEmitterIsDone = false;
}

/**
 *	Retrieved the per-particle bytes that this emitter type requires.
 *
 *	@return	uint32	The number of required bytes for particles in the instance
 */
uint32 FParticleEmitterInstance::RequiredBytes()
{
    // If ANY LOD level has subUV, the size must be taken into account.
    uint32 uiBytes = 0;
    bool bHasSubUV = false;
    for (int32 LODIndex = 0; (LODIndex < SpriteTemplate->LODLevels.Num()) && !bHasSubUV; LODIndex++)
    {
        // This code assumes that the module stacks are identical across LOD levevls...
        UParticleLODLevel* LODLevel = SpriteTemplate->GetLODLevel(LODIndex);

        if (LODLevel)
        {
            EParticleSubUVInterpMethod	InterpolationMethod = (EParticleSubUVInterpMethod)LODLevel->RequiredModule->InterpolationMethod;
            //if (LODIndex > 0)
            //{
            //    if ((InterpolationMethod != PSUVIM_None) && (bHasSubUV == false))
            //    {
            //        UE_LOG(LogParticles, Warning, TEXT("Emitter w/ mismatched SubUV settings: %s"),
            //            Component ?
            //            Component->Template ?
            //            *(Component->Template->GetPathName()) :
            //            *(Component->GetFullName()) :
            //            TEXT("INVALID PSYS!"));
            //    }

            //    if ((InterpolationMethod == PSUVIM_None) && (bHasSubUV == true))
            //    {
            //        UE_LOG(LogParticles, Warning, TEXT("Emitter w/ mismatched SubUV settings: %s"),
            //            Component ?
            //            Component->Template ?
            //            *(Component->Template->GetPathName()) :
            //            *(Component->GetFullName()) :
            //            TEXT("INVALID PSYS!"));
            //    }
            //}
            
            // Check for SubUV utilization, and update the required bytes accordingly
            if (InterpolationMethod != PSUVIM_None)
            {
                bHasSubUV = true;
            }
        }
    }

    if (bHasSubUV)
    {
        SubUVDataOffset = PayloadOffset;
        uiBytes = sizeof(FFullSubUVPayload);
    }

    return uiBytes;
}

/**
 *	Calculate the stride of a single particle for this instance
 *
 *	@param	ParticleSize	The size of the particle
 *
 *	@return	uint32			The stride of the particle
 */
uint32 FParticleEmitterInstance::CalculateParticleStride(uint32 InParticleSize)
{
    return InParticleSize;
}
