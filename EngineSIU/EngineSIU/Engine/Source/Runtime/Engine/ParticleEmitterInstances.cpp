#include "ParticleEmitterInstances.h"

#include "Define.h"
#include "Engine/Engine.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/Event/ParticleModuleEventGenerator.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"

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
	if (NewMaxActiveParticles > MaxActiveParticles)
	{
		{
		    ParticleData = (uint8*) FPlatformMemory::Realloc<EAT_Container>(ParticleData, ParticleStride * NewMaxActiveParticles);

			// Allocate memory for indices.
			if (ParticleIndices == NULL)
			{
				// Make sure that we clear all when it is the first alloc
				MaxActiveParticles = 0;
			}
		    ParticleIndices = (uint16*) FPlatformMemory::Realloc<EAT_Container>(ParticleIndices, sizeof(uint16) * (NewMaxActiveParticles + 1));
		}

		// Fill in default 1:1 mapping.
		for (int32 i=MaxActiveParticles; i<NewMaxActiveParticles; i++)
		{
			ParticleIndices[i] = i;
		}

		// Set the max count
		MaxActiveParticles = NewMaxActiveParticles;
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
	if ((SpawnRate > 0.f) || (BurstCount > 0))
	{
		float SafetyLeftover = OldLeftover;
		// Ensure continuous spawning... lots of fiddling.
		float	NewLeftover = OldLeftover + DeltaTime * SpawnRate;
		int32		Number		= FMath::FloorToInt(NewLeftover);
		float	Increment	= (SpawnRate > 0.0f) ? (1.f / SpawnRate) : 0.0f;
		float	StartTime = DeltaTime + OldLeftover * Increment - Increment;
		NewLeftover			= NewLeftover - Number;
	
		// Handle growing arrays.
		bool bProcessSpawn = true;
		int32 NewCount = ActiveParticles + Number + BurstCount;
	
		// float	BurstIncrement = SpriteTemplate->bUseLegacySpawningBehavior ? (BurstCount > 0.0f) ? (1.f / BurstCount) : 0.0f : 0.0f;
		// float	BurstStartTime = SpriteTemplate->bUseLegacySpawningBehavior ? DeltaTime * BurstIncrement : 0.0f;
	
		if (NewCount >= MaxActiveParticles)
		{
            bProcessSpawn = Resize(NewCount + FMath::FloorToInt(FMath::Sqrt(FMath::Sqrt((float)NewCount)) + 1));
		}
	
		if (bProcessSpawn == true)
		{
			FParticleEventInstancePayload* EventPayload = NULL;
			if (LODLevel->EventGenerator)
			{
				EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
				if (EventPayload && !EventPayload->bSpawnEventsPresent && !EventPayload->bBurstEventsPresent)
				{
					EventPayload = NULL;
				}
			}
	
			const FVector InitialLocation = EmitterToSimulation.GetTranslationVector();
	
			// Spawn particles.
			SpawnParticles( Number, StartTime, Increment, InitialLocation, FVector::ZeroVector, EventPayload );
	
			// Burst particles.
			// SpawnParticles(BurstCount, BurstStartTime, BurstIncrement, InitialLocation, FVector::ZeroVector, EventPayload);
	
			return NewLeftover;
		}
		return SafetyLeftover;
	}

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
            return;
        }
        
        UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
        float SpawnTime = StartTime;
        float Interp = 1.0f;
        const float InterpIncrement = (Count > 0 && Increment > 0.0f) ? (1.0f / static_cast<float>(Count)) : 0.0f;
        for (int32 i = 0; i < Count; ++i)
        {
            uint16 NextFreeIndex = ParticleIndices[ActiveParticles];
            DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * NextFreeIndex);
            const uint32 CurrentParticleIndex = ActiveParticles++;

            PreSpawn(Particle, InitialLocation, InitialVelocity);
            for (int32 ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ++ModuleIndex)
            {
                UParticleModule* SpawnModule = LODLevel->SpawnModules[ModuleIndex];
                if (SpawnModule->GetFlag(EModuleFlag::Enabled))
                {
                    UParticleModule* OffsetModule = HighestLODLevel->SpawnModules[ModuleIndex];
                    SpawnModule->Spawn(this, GetModuleDataOffset(OffsetModule), SpawnTime, Particle);
                }
            }
            PostSpawn(Particle, Interp, SpawnTime);

            if (Particle->RelativeTime > 1.0f)
            {
                KillParticle(CurrentParticleIndex);
                continue;
            }
        }

        // SpawnInternal();
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

/**
 * Captures dynamic replay data for this particle system.
 *
 * @param	OutData		[Out] Data will be copied here
 *
 * @return Returns true if successful
 */
// bool FParticleEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
// {
//     QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleEmitterInstance_FillReplayData);
//
// 	// NOTE: This the base class implementation that should ONLY be called by derived classes' FillReplayData()!
//
// 	// Make sure there is a template present
// 	if (!SpriteTemplate)
// 	{
// 		return false;
// 	}
//
// 	// Allocate it for now, but we will want to change this to do some form
// 	// of caching
// 	if (ActiveParticles <= 0 || !bEnabled)
// 	{
// 		return false;
// 	}
// 	// If the template is disabled, don't return data.
// 	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
// 	if ((LODLevel == NULL) || (LODLevel->bEnabled == false))
// 	{
// 		return false;
// 	}
//
// 	// Make sure we will not be allocating enough memory
// 	check(MaxActiveParticles >= ActiveParticles);
//
// 	// Must be filled in by implementation in derived class
// 	OutData.eEmitterType = DET_Unknown;
//
// 	OutData.ActiveParticleCount = ActiveParticles;
// 	OutData.ParticleStride = ParticleStride;
// 	OutData.SortMode = SortMode;
//
// 	// Take scale into account
// 	OutData.Scale = FVector3f::OneVector;
// 	if (Component)
// 	{
// 		OutData.Scale = FVector3f(Component->GetComponentTransform().GetScale3D());
// 	}
//
// 	int32 ParticleMemSize = MaxActiveParticles * ParticleStride;
//
// 	// Allocate particle memory
//
// 	OutData.DataContainer.Alloc(ParticleMemSize, MaxActiveParticles);
// 	INC_DWORD_STAT_BY(STAT_RTParticleData, OutData.DataContainer.MemBlockSize);
//
// 	FMemory::BigBlockMemcpy(OutData.DataContainer.ParticleData, ParticleData, ParticleMemSize);
// 	FMemory::Memcpy(OutData.DataContainer.ParticleIndices, ParticleIndices, OutData.DataContainer.ParticleIndicesNumShorts * sizeof(uint16));
//
// 	// All particle emitter types derived from sprite emitters, so we can fill that data in here too!
// 	{
// 		FDynamicSpriteEmitterReplayDataBase* NewReplayData =
// 			static_cast< FDynamicSpriteEmitterReplayDataBase* >( &OutData );
//
// 		NewReplayData->RequiredModule = LODLevel->RequiredModule->CreateRendererResource();
// 		NewReplayData->MaterialInterface = NULL;	// Must be set by derived implementation
// 		NewReplayData->InvDeltaSeconds = (LastDeltaTime > UE_KINDA_SMALL_NUMBER) ? (1.0f / LastDeltaTime) : 0.0f;
// 		NewReplayData->LWCTile = ((Component == nullptr) || LODLevel->RequiredModule->bUseLocalSpace) ? FVector3f::Zero() : Component->GetLWCTile();
//
// 		NewReplayData->MaxDrawCount =
// 			(LODLevel->RequiredModule->bUseMaxDrawCount == true) ? LODLevel->RequiredModule->MaxDrawCount : -1;
// 		NewReplayData->ScreenAlignment	= LODLevel->RequiredModule->ScreenAlignment;
// 		NewReplayData->bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
// 		NewReplayData->EmitterRenderMode = SpriteTemplate->EmitterRenderMode;
// 		NewReplayData->DynamicParameterDataOffset = DynamicParameterDataOffset;
// 		NewReplayData->LightDataOffset = LightDataOffset;
// 		NewReplayData->LightVolumetricScatteringIntensity = LightVolumetricScatteringIntensity;
// 		NewReplayData->CameraPayloadOffset = CameraPayloadOffset;
//
// 		NewReplayData->SubUVDataOffset = SubUVDataOffset;
// 		NewReplayData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
// 		NewReplayData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
//
// 		NewReplayData->MacroUVOverride.bOverride = LODLevel->RequiredModule->bOverrideSystemMacroUV;
// 		NewReplayData->MacroUVOverride.Radius = LODLevel->RequiredModule->MacroUVRadius;
// 		NewReplayData->MacroUVOverride.Position = FVector3f(LODLevel->RequiredModule->MacroUVPosition);
//         
// 		NewReplayData->bLockAxis = false;
// 		if (bAxisLockEnabled == true)
// 		{
// 			NewReplayData->LockAxisFlag = LockAxisFlags;
// 			if (LockAxisFlags != EPAL_NONE)
// 			{
// 				NewReplayData->bLockAxis = true;
// 			}
// 		}
//
// 		// If there are orbit modules, add the orbit module data
// 		if (LODLevel->OrbitModules.Num() > 0)
// 		{
// 			UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
// 			UParticleModuleOrbit* LastOrbit = HighestLODLevel->OrbitModules[LODLevel->OrbitModules.Num() - 1];
// 			check(LastOrbit);
//
// 			uint32* LastOrbitOffset = SpriteTemplate->ModuleOffsetMap.Find(LastOrbit);
// 			NewReplayData->OrbitModuleOffset = *LastOrbitOffset;
// 		}
//
// 		NewReplayData->EmitterNormalsMode = LODLevel->RequiredModule->EmitterNormalsMode;
// 		NewReplayData->NormalsSphereCenter = (FVector3f)LODLevel->RequiredModule->NormalsSphereCenter;
// 		NewReplayData->NormalsCylinderDirection = (FVector3f)LODLevel->RequiredModule->NormalsCylinderDirection;
//
// 		NewReplayData->PivotOffset = FVector2f(PivotOffset);
//
// 		NewReplayData->bUseVelocityForMotionBlur = LODLevel->RequiredModule->ShouldUseVelocityForMotionBlur();
// 		NewReplayData->bRemoveHMDRoll = LODLevel->RequiredModule->bRemoveHMDRoll;
// 		NewReplayData->MinFacingCameraBlendDistance = LODLevel->RequiredModule->MinFacingCameraBlendDistance;
// 		NewReplayData->MaxFacingCameraBlendDistance = LODLevel->RequiredModule->MaxFacingCameraBlendDistance;
// 	}
//
//
// 	return true;
// }

/**
 *	Retrieves the dynamic data for the emitter
 *	
 *	@param	bSelected					Whether the emitter is selected in the editor
 *
 *	@return	FDynamicEmitterDataBase*	The dynamic data, or NULL if it shouldn't be rendered
 */
// FDynamicEmitterDataBase* FParticleMeshEmitterInstance::GetDynamicData(bool bSelected)
// {
//     QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleMeshEmitterInstance_GetDynamicData);
//
//     // It is safe for LOD level to be NULL here!
//     UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
//     // if (IsDynamicDataRequired(LODLevel) == false || !bEnabled)
//     // {
//     //     return nullptr;
//     // }
//
//     // Allocate the dynamic data
//     FDynamicMeshEmitterData* NewEmitterData = new FDynamicMeshEmitterData(LODLevel->RequiredModule);
//     {
//         // INC_DWORD_STAT(STAT_DynamicEmitterCount);
//         // INC_DWORD_STAT(STAT_DynamicMeshCount);
//         // INC_DWORD_STAT_BY(STAT_DynamicEmitterMem, sizeof(FDynamicMeshEmitterData));
//     }
//
//     // Now fill in the source data
//     if( !FillReplayData( NewEmitterData->Source ) )
//     {
//         delete NewEmitterData;
//         return nullptr;
//     }
//
//
//     // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
//     NewEmitterData->Init(
//         this,
//         MeshTypeData->Mesh,
//         MeshTypeData->LODSizeScale
//     );
//
//     return NewEmitterData;
// }

bool FParticleMeshEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleMeshEmitterInstance_FillReplayData);

	// Call parent implementation first to fill in common particle source data
	if( !FParticleEmitterInstance::FillReplayData( OutData ) )
	{
		return false;
	}

	// Grab the LOD level
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == false))
	{
		return false;
	}

	OutData.eEmitterType = DET_Mesh;

	FDynamicMeshEmitterReplayData* NewReplayData = static_cast< FDynamicMeshEmitterReplayData* >( &OutData );

	// UMaterialInterface* RenderMaterial = CurrentMaterial;
	// if (RenderMaterial == NULL  || (RenderMaterial->CheckMaterialUsage_Concurrent(MATUSAGE_MeshParticles) == false))
	// {
	// 	RenderMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
	// }
	// NewReplayData->MaterialInterface = RenderMaterial;
	// CurrentMaterial = RenderMaterial;
	//
	// // Mesh settings
	// NewReplayData->bScaleUV = LODLevel->RequiredModule->bScaleUV;
	// NewReplayData->SubUVInterpMethod = LODLevel->RequiredModule->InterpolationMethod;
	// NewReplayData->SubUVDataOffset = SubUVDataOffset;
	// NewReplayData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
	// NewReplayData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
	// NewReplayData->MeshRotationOffset = MeshRotationOffset;
	// NewReplayData->MeshMotionBlurOffset = MeshMotionBlurOffset;
	// NewReplayData->bMeshRotationActive = MeshRotationActive;
	// NewReplayData->MeshAlignment = MeshTypeData->MeshAlignment;
	//
	// // Scale needs to be handled in a special way for meshes.  The parent implementation set this
	// // itself, but we'll recompute it here.
	// NewReplayData->Scale = FVector::OneVector;
	// if (Component)
	// {
	// 	check(SpriteTemplate);
	// 	UParticleLODLevel* LODLevel2 = SpriteTemplate->GetCurrentLODLevel(this);
	// 	check(LODLevel2);
	// 	check(LODLevel2->RequiredModule);
	// 	// Take scale into account
	// 	if (LODLevel2->RequiredModule->bUseLocalSpace == false)
	// 	{
	// 		if (!bIgnoreComponentScale)
	// 		{
	// 			NewReplayData->Scale = Component->GetWorldScale3D();
	// 		}
	// 	}
	// }
	//
	// // See if the new mesh locked axis is being used...
	// if (MeshTypeData->AxisLockOption == EPAL_NONE)
	// {
	// 	if (bAxisLockEnabled)
	// 	{
	// 		NewReplayData->LockAxisFlag = LockAxisFlags;
	// 		if (LockAxisFlags != EPAL_NONE)
	// 		{
	// 			NewReplayData->bLockAxis = true;
	// 			switch (LockAxisFlags)
	// 			{
	// 			case EPAL_X:
	// 				NewReplayData->LockedAxis = FVector3f(1,0,0);
	// 				break;
	// 			case EPAL_Y:
	// 				NewReplayData->LockedAxis = FVector3f(0,1,0);
	// 				break;
	// 			case EPAL_NEGATIVE_X:
	// 				NewReplayData->LockedAxis = FVector3f(-1,0,0);
	// 				break;
	// 			case EPAL_NEGATIVE_Y:
	// 				NewReplayData->LockedAxis = FVector3f(0,-1,0);
	// 				break;
	// 			case EPAL_NEGATIVE_Z:
	// 				NewReplayData->LockedAxis = FVector3f(0,0,-1);
	// 				break;
	// 			case EPAL_Z:
	// 			case EPAL_NONE:
	// 			default:
	// 				NewReplayData->LockedAxis = FVector3f(0,0,1);
	// 				break;
	// 			}
	// 		}
	// 	}
	// }

	return true;
}

class UParticleLODLevel* FParticleEmitterInstance::GetCurrentLODLevelChecked()
{
    if (SpriteTemplate == nullptr)
    {
        return nullptr;
    }
    
    UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    if (LODLevel == nullptr)
    {
        return nullptr;
    }
    
    if (LODLevel->RequiredModule == nullptr)
    {
        return nullptr;
    }
    return LODLevel;
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
