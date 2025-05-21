#include "ParticleEmitterInstances.h"

#include "Define.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/Event/ParticleModuleEventGenerator.h"
#include "RandomStream.h"
#include "Components/Material/Material.h"
#include "Components/Mesh/StaticMeshRenderData.h"
#include "Core/HAL/PlatformMemory.h"
#include "Templates/AlignmentTemplates.h"
#include "Runtime/Engine/World/World.h"

#include "Particles/ParticleModuleSpawn.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"
#include "Particles/ParticleModuleSpawn.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/Casts.h"

FORCEINLINE static void* FastParticleSmallBlockAlloc(size_t AllocSize)
{
    return FPlatformMemory::Malloc<EAT_Container>(AllocSize);
}

FORCEINLINE static void FastParticleSmallBlockFree(void *RawMemory, size_t AllocSize)
{
    FPlatformMemory::Free<EAT_Container>(RawMemory, AllocSize);
}

void FParticleDataContainer::Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts)
{
    assert(InParticleDataNumBytes > 0 && ParticleIndicesNumShorts >= 0
        && InParticleDataNumBytes % sizeof(uint16) == 0); // we assume that the particle storage has reasonable alignment below
    ParticleDataNumBytes = InParticleDataNumBytes;
    ParticleIndicesNumShorts = InParticleIndicesNumShorts;

    MemBlockSize = ParticleDataNumBytes + ParticleIndicesNumShorts * sizeof(uint16);

    ParticleData = (uint8*)FastParticleSmallBlockAlloc(MemBlockSize);
    ParticleIndices = (uint16*)(ParticleData + ParticleDataNumBytes);
}

void FParticleDataContainer::Free()
{
    if (ParticleData)
    {
        assert(MemBlockSize > 0);
        FastParticleSmallBlockFree(ParticleData, MemBlockSize);
    }
    MemBlockSize = 0;
    ParticleDataNumBytes = 0;
    ParticleIndicesNumShorts = 0;
    ParticleData = nullptr;
    ParticleIndices = nullptr;
}


void FParticleEmitterInstance::ResetParticleParameters(float DeltaTime)
{
    UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
    assert(HighestLODLevel);

    // Store off any orbit offset values
    TArray<int32> OrbitOffsets;
    // int32 OrbitCount = LODLevel->OrbitModules.Num();
    // for (int32 OrbitIndex = 0; OrbitIndex < OrbitCount; OrbitIndex++)
    // {
    //     UParticleModuleOrbit* OrbitModule = HighestLODLevel->OrbitModules[OrbitIndex];
    //     if (OrbitModule)
    //     {
    //         uint32* OrbitOffset = SpriteTemplate->ModuleOffsetMap.Find(OrbitModule);
    //         if (OrbitOffset)
    //         {
    //             OrbitOffsets.Add(*OrbitOffset);
    //         }
    //     }
    // }

    //bool bSkipDoubleSpawnUpdate = !SpriteTemplate->bUseLegacySpawningBehavior;
    bool bSkipDoubleSpawnUpdate = true;
    for (int32 ParticleIndex = 0; ParticleIndex < ActiveParticles; ParticleIndex++)
    {
        DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[ParticleIndex]);
        Particle.Velocity		= Particle.BaseVelocity;
        Particle.Size = GetParticleBaseSize(Particle);
        Particle.RotationRate	= Particle.BaseRotationRate;
        Particle.Color = Particle.BaseColor;
    
        bool bJustSpawned = (Particle.Flags & STATE_Particle_JustSpawned) != 0;
    
        //Don't update position for newly spawned particles. They already have a partial update applied during spawn.
        bool bSkipUpdate = bJustSpawned && bSkipDoubleSpawnUpdate;
    
        Particle.RelativeTime	+= bSkipUpdate ? 0.0f : Particle.OneOverMaxLifetime * DeltaTime;
    
        if (CameraPayloadOffset > 0)
        {
            int32 CurrentOffset = CameraPayloadOffset;
            const uint8* ParticleBase = (const uint8*)&Particle;
            // PARTICLE_ELEMENT(FCameraOffsetParticlePayload, CameraOffsetPayload);
            // CameraOffsetPayload.Offset = CameraOffsetPayload.BaseOffset;
        }
        for (int32 OrbitIndex = 0; OrbitIndex < OrbitOffsets.Num(); OrbitIndex++)
        {
            int32 CurrentOffset = OrbitOffsets[OrbitIndex];
            const uint8* ParticleBase = (const uint8*)&Particle;
            // PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
            // OrbitPayload.PreviousOffset = OrbitPayload.Offset;
            // OrbitPayload.Offset = OrbitPayload.BaseOffset;
            // OrbitPayload.RotationRate = OrbitPayload.BaseRotationRate;
        }
    }
}

void FParticleEmitterInstance::KillParticles()
{
    if (ActiveParticles <= 0)
    {
        return;
    }
    UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    FParticleEventInstancePayload* EventPayload = NULL;
    // if (LODLevel->EventGenerator)
    // {
    //     EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
    //     if (EventPayload && (EventPayload->bDeathEventsPresent == false))
    //     {
    //         EventPayload = NULL;
    //     }
    // }
    //
    bool bFoundCorruptIndices = false;
    // Loop over the active particles... If their RelativeTime is > 1.0f (indicating they are dead),
    // move them to the 'end' of the active particle list.
    for (int32 i=ActiveParticles-1; i>=0; i--)
    {
        const int32	CurrentIndex	= ParticleIndices[i];
        if (CurrentIndex < MaxActiveParticles)
        { 
            const uint8* ParticleBase = ParticleData + CurrentIndex * ParticleStride;
            FBaseParticle& Particle = *((FBaseParticle*)ParticleBase);
    
            if (Particle.RelativeTime > 1.0f)
            {
                if (EventPayload)
                {
                    // LODLevel->EventGenerator->HandleParticleKilled(this, EventPayload, &Particle);
                }
                // Move it to the 'back' of the list
                ParticleIndices[i] = ParticleIndices[ActiveParticles-1];
                ParticleIndices[ActiveParticles-1]	= CurrentIndex;
                ActiveParticles--;
    
                // INC_DWORD_STAT(STAT_SpriteParticlesKilled);
            }
        }
        else
        {
            bFoundCorruptIndices = true;
        }
        
        if (bFoundCorruptIndices)
        {
            UE_LOG(LogLevel::Error, TEXT("Detected corrupted particle indices. Template : %s, Component %s"), *(Component && Component ? Component->Template : nullptr)->GetName(), *(Component)->GetName());			
            FixupParticleIndices();
        }
    }
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
    //if (GEngine->MaxParticleResize > 0)
    {
        if ((NewMaxActiveParticles < 0)
            //|| (NewMaxActiveParticles > GEngine->MaxParticleResize)
            )
        {
            if ((NewMaxActiveParticles < 0)
                //|| (NewMaxActiveParticles > GEngine->MaxParticleResizeWarn)
                )
            {
                UE_LOG(LogLevel::Warning, TEXT("Emitter::Resize> Invalid NewMaxActive (%d) for Emitter in PSys %s"),
                    NewMaxActiveParticles,
                    Component	? 
                                Component->Template ? *(Component->Template->GetName())
                                                    : *(Component->GetName()) 
                                :
                                TEXT("INVALID COMPONENT"));
            }

            return false;
        }
    }
    // Check Execption
    
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
    assert(SpriteTemplate);
    assert(SpriteTemplate->LODLevels.Num() > 0);

    bool bFirstTime = (SecondsSinceCreation > 0.f) ? false : true;
    
    // Grab the current LOD level
    UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    // Handle EmitterTime setup, looping, etc.
    float EmitterDelay = Tick_EmitterTimeSetup(DeltaTime, LODLevel);

    if (bEnabled)
    {
        KillParticles();
        
        ResetParticleParameters(DeltaTime);
        
        CurrentMaterial = LODLevel->RequiredModule->Material;
        Tick_ModuleUpdate(DeltaTime, LODLevel);
        
        SpawnFraction = Tick_SpawnParticles(DeltaTime, LODLevel, bSuppressSpawning, bFirstTime);
        
        if (ActiveParticles > 0)
        {
            // Update the orbit data...
            // UpdateOrbitData(DeltaTime);
            // Calculate bounding box and simulate velocity.
            UpdateBoundingBox(DeltaTime);
        }

        Tick_ModuleFinalUpdate(DeltaTime, LODLevel);

        // CheckEmitterFinished();

        // Invalidate the contents of the vertex/index buffer.
        IsRenderDataDirty = 1;
    }

    EmitterTime += EmitterDelay;
}

float FParticleEmitterInstance::Tick_EmitterTimeSetup(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
    //Make sure we don't try and do any interpolation on the first frame we are attached (OldLocation is not valid in this circumstance)
	// if (Component->bJustRegistered)
	// {
	// 	Location	= Component->GetComponentLocation();
	// 	OldLocation	= Location;
	// }
	// else
	{
		// Keep track of location for world- space interpolation and other effects.
		OldLocation	= Location;
		Location	= Component->GetWorldLocation();
	}
    
	UpdateTransforms();
	SecondsSinceCreation += DeltaTime;

	// Update time within emitter loop.
	bool bLooped = false;
	// if (InCurrentLODLevel->RequiredModule->bUseLegacyEmitterTime == false)
	{
		EmitterTime += DeltaTime;
		bLooped = (EmitterDuration > 0.0f) && (EmitterTime >= EmitterDuration);
	}
	// else
	// {
	// 	EmitterTime = SecondsSinceCreation;
	// 	if (EmitterDuration > KINDA_SMALL_NUMBER)
	// 	{
	// 		EmitterTime = FMath::Fmod(SecondsSinceCreation, EmitterDuration);
	// 		bLooped = ((SecondsSinceCreation - (EmitterDuration * LoopCount)) >= EmitterDuration);
	// 	}
	// }

	// Get the emitter delay time
	float EmitterDelay = CurrentDelay;

	// Determine if the emitter has looped
	if (bLooped)
	{
		LoopCount++;
		// ResetBurstList();
// #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
// 		// Reset the event count each loop...
// 		if (EventCount > MaxEventCount)
// 		{
// 			MaxEventCount = EventCount;
// 		}
// 		EventCount = 0;
// #endif	//#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

		// if (InCurrentLODLevel->RequiredModule->bUseLegacyEmitterTime == false)
		{
			EmitterTime -= EmitterDuration;
		}

		if ((InCurrentLODLevel->RequiredModule->bDurationRecalcEachLoop == true)
			|| ((InCurrentLODLevel->RequiredModule->bDelayFirstLoopOnly == true) && (LoopCount == 1))
			)
		{
			SetupEmitterDuration();
		}

		// if (bRequiresLoopNotification == true)
		// {
		// 	for (int32 ModuleIdx = -3; ModuleIdx < InCurrentLODLevel->Modules.Num(); ModuleIdx++)
		// 	{
		// 		int32 ModuleFetchIdx;
		// 		switch (ModuleIdx)
		// 		{
		// 		case -3:	ModuleFetchIdx = INDEX_REQUIREDMODULE;	break;
		// 		case -2:	ModuleFetchIdx = INDEX_SPAWNMODULE;		break;
		// 		case -1:	ModuleFetchIdx = INDEX_TYPEDATAMODULE;	break;
		// 		default:	ModuleFetchIdx = ModuleIdx;				break;
		// 		}
		//
		// 		UParticleModule* Module = InCurrentLODLevel->GetModuleAtIndex(ModuleFetchIdx);
		// 		if (Module != NULL)
		// 		{
		// 			if (Module->GetFlag(EModuleFlag::RequiresLoopingNotification))
		// 			{
		// 				Module->EmitterLoopingNotify(this);
		// 			}
		// 		}
		// 	}
		// }
	}

	// Don't delay unless required
	if ((InCurrentLODLevel->RequiredModule->bDelayFirstLoopOnly == true) && (LoopCount > 0))
	{
		EmitterDelay = 0;
	}

	// 'Reset' the emitter time so that the modules function correctly
	EmitterTime -= EmitterDelay;

	return EmitterDelay;
}

float FParticleEmitterInstance::Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* InCurrentLODLevel, bool bSuppressSpawning, bool bFirstTime)
{
    if (!bHaltSpawning && !bHaltSpawningExternal && !bSuppressSpawning && (EmitterTime >= 0.0f))
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
        if (CurrentModule &&
            // CurrentModule->GetFlag(EModuleFlag::Enabled) &&
            CurrentModule->GetFlag(EModuleFlag::UpdateModule))
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
        if (CurrentModule &&
            // CurrentModule->GetFlag(EModuleFlag::Enabled) &&
            CurrentModule->GetFlag(EModuleFlag::UpdateModule))
        {
            CurrentModule->FinalUpdate(this, GetModuleDataOffset(HighestLODLevel->UpdateModules[ModuleIndex]), DeltaTime);
        }
    }
}

void FParticleEmitterInstance::UpdateBoundingBox(float DeltaTime)
{
    // SCOPE_CYCLE_COUNTER(STAT_ParticleUpdateBounds);
	if (Component)
	{
		bool bUpdateBox =
		    // ((Component->bWarmingUp == false) &&
		        (Component->Template != NULL)
		        // && (Component->Template->bUseFixedRelativeBoundingBox == false))
	    ;

		// Take component scale into account
		FVector Scale = Component->GetComponentTransform().GetScale3D();

		UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

		FVector	NewLocation;
		float	NewRotation;
		if (bUpdateBox)
		{
			// ParticleBoundingBox.Init();
		}
		UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
		assert(HighestLODLevel);

		FVector ParticlePivotOffset(-0.5f,-0.5f,0.0f);
		if( bUpdateBox )
		{
			uint32 NumModules = HighestLODLevel->Modules.Num();
			// for( uint32 i = 0; i < NumModules; ++i )
			// {
			// 	UParticleModulePivotOffset* Module = Cast<UParticleModulePivotOffset>(HighestLODLevel->Modules[i]);
			// 	if( Module )
			// 	{
			// 		FVector2D PivotOff = Module->PivotOffset;
			// 		ParticlePivotOffset += FVector(PivotOff.X, PivotOff.Y, 0.0f);
			// 		break;
			// 	}
			// }
		}

		// Store off the orbit offset, if there is one
		// int32 OrbitOffsetValue = GetOrbitPayloadOffset();

		// For each particle, offset the box appropriately 
		// FVector MinVal(HALF_WORLD_MAX);
		// FVector MaxVal(-HALF_WORLD_MAX);
		
		const bool bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;

		const FMatrix ComponentToWorld = bUseLocalSpace 
			? Component->GetWorldMatrix() 
			: FMatrix::Identity;

		// bool bSkipDoubleSpawnUpdate = !SpriteTemplate->bUseLegacySpawningBehavior;
		for (int32 i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			
			// Do linear integrator and update bounding box
			// Do angular integrator, and wrap result to within +/- 2 PI
			Particle.OldLocation	= Particle.Location;

			bool bJustSpawned = (Particle.Flags & STATE_Particle_JustSpawned) != 0;
			Particle.Flags &= ~STATE_Particle_JustSpawned;

			//Don't update position for newly spawned particles. They already have a partial update applied during spawn.
			bool bSkipUpdate = bJustSpawned
		    // && bSkipDoubleSpawnUpdate
		    ;

			if ((Particle.Flags & STATE_Particle_Freeze) == 0 && !bSkipUpdate)
			{
				if ((Particle.Flags & STATE_Particle_FreezeTranslation) == 0)
				{
					NewLocation = Particle.Location + FVector(Particle.Velocity) * DeltaTime;
				}
				else
				{
					NewLocation = Particle.Location;
				}
				if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
				{
					NewRotation = (DeltaTime * Particle.RotationRate) + Particle.Rotation;
				}
				else
				{
					NewRotation = Particle.Rotation;
				}
			}
			else
			{
				NewLocation = Particle.Location;
				NewRotation = Particle.Rotation;
			}

			// FVector::FReal LocalMax(0.0f);

			if (bUpdateBox)
			{	
				// if (OrbitOffsetValue == -1)
				// {
				// 	LocalMax = (FVector(Particle.Size) * Scale).GetAbsMax();
				// }
				// else
				{
					// int32 CurrentOffset = OrbitOffsetValue;
					const uint8* ParticleBase = (const uint8*)&Particle;
					// PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
					// LocalMax = OrbitPayload.Offset.GetAbsMax();
				}

				// LocalMax += (FVector(Particle.Size) * ParticlePivotOffset).GetAbsMax();
			}

			// NewLocation			+= PositionOffsetThisTick;
			// Particle.OldLocation+= PositionOffsetThisTick;
						
			Particle.Location	 = NewLocation;
			Particle.Rotation	 = FMath::Fmod(NewRotation, 2.f*(float)PI);

			if (bUpdateBox)
			{	
				FVector PositionForBounds = NewLocation;

				if (bUseLocalSpace)
				{
					// Note: building the bounding box in world space as that gives tighter bounds than transforming a local space AABB into world space
					PositionForBounds = ComponentToWorld.TransformPosition(NewLocation);
				}

				// Treat each particle as a cube whose sides are the length of the maximum component
				// This handles the particle's extents changing due to being camera facing
				// MinVal[0] = FMath::Min(MinVal[0], PositionForBounds.X - LocalMax);
				// MaxVal[0] = FMath::Max(MaxVal[0], PositionForBounds.X + LocalMax);
				// MinVal[1] = FMath::Min(MinVal[1], PositionForBounds.Y - LocalMax);
				// MaxVal[1] = FMath::Max(MaxVal[1], PositionForBounds.Y + LocalMax);
				// MinVal[2] = FMath::Min(MinVal[2], PositionForBounds.Z - LocalMax);
				// MaxVal[2] = FMath::Max(MaxVal[2], PositionForBounds.Z + LocalMax);
			}
		}

		if (bUpdateBox)
		{
			// ParticleBoundingBox = FBox(MinVal, MaxVal);
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
	// int32 BurstCount = 0;
	float SpawnRateDivisor = 0.0f;
	float OldLeftover = SpawnFraction;

	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];

	bool bProcessSpawnRate = true;
	bool bProcessBurstList = true;
	

	// Process all Spawning modules that are present in the emitter.
	for (int32 SpawnModIndex = 0; SpawnModIndex < LODLevel->SpawningModules.Num(); SpawnModIndex++)
	{
		UParticleModuleSpawnBase* SpawnModule = LODLevel->SpawningModules[SpawnModIndex];
		if (SpawnModule
		    // && SpawnModule->GetFlag(EModuleFlag::Enabled)
		    )
		{
			UParticleModule* OffsetModule = HighestLODLevel->SpawningModules[SpawnModIndex];
			uint32 Offset = GetModuleDataOffset(OffsetModule);

			// Update the spawn rate
			int32 Number = 0;
			float Rate = 0.0f;
			if (SpawnModule->GetSpawnAmount(this, Offset, OldLeftover, DeltaTime, Number, Rate) == false)
			{
				bProcessSpawnRate = false;
			}

			Number = FMath::Max<int32>(0, Number);
			Rate = FMath::Max<float>(0.0f, Rate);

			SpawnCount += Number;
			SpawnRate += Rate;
			// Update the burst list
			// int32 BurstNumber = 0;
			// if (SpawnModule->GetBurstCount(this, Offset, OldLeftover, DeltaTime, BurstNumber) == false)
			// {
			// 	bProcessBurstList = false;
			// }
			//
			// BurstCount += BurstNumber;
		}
	}

    if (bProcessSpawnRate)
    {
        float RateScale = LODLevel->SpawnModule->RateScale.GetValue(EmitterTime) * LODLevel->SpawnModule->GetGlobalRateScale();
        SpawnRate += LODLevel->SpawnModule->Rate.GetValue(EmitterTime) * RateScale;
        SpawnRate = FMath::Max<float>(0.0f, SpawnRate);
    }
    
	// Take Bursts into account as well...
	// if (bProcessBurstList)
	// {
	// 	int32 Burst = 0;
	// 	float BurstTime = GetCurrentBurstRateOffset(DeltaTime, Burst);
	// 	BurstCount += Burst;
	// }

	// float QualityMult = SpriteTemplate->GetQualityLevelSpawnRateMult();
    float QualityMult = 1.0f;
	SpawnRate = FMath::Max<float>(0.0f, SpawnRate * QualityMult);
	// BurstCount = FMath::CeilToInt(BurstCount * QualityMult);


	// Spawn new particles...
	if ((SpawnRate > 0.f)
	    // || (BurstCount > 0)
	    )
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
		int32 NewCount = ActiveParticles + Number
	    // + BurstCount
	    ;
	
		// float	BurstIncrement = SpriteTemplate->bUseLegacySpawningBehavior ? (BurstCount > 0.0f) ? (1.f / BurstCount) : 0.0f : 0.0f;
		// float	BurstStartTime = SpriteTemplate->bUseLegacySpawningBehavior ? DeltaTime * BurstIncrement : 0.0f;
	
		if (NewCount >= MaxActiveParticles)
		{
            bProcessSpawn = Resize(NewCount + FMath::FloorToInt(FMath::Sqrt(FMath::Sqrt((float)NewCount)) + 1));
		}
	
		if (bProcessSpawn == true)
		{
			FParticleEventInstancePayload* EventPayload = NULL;
			// if (LODLevel->EventGenerator)
			// {
			// 	EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
			// 	if (EventPayload && !EventPayload->bSpawnEventsPresent && !EventPayload->bBurstEventsPresent)
			// 	{
			// 		EventPayload = NULL;
			// 	}
			// }
	
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
    static float time = 0;
    time += FMath::Abs(StartTime);

	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

    assert(ActiveParticles <= MaxActiveParticles);
    // assert(LODLevel->EventGenerator != NULL || EventPayload == NULL);

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
            // Workaround to corrupted indices.
            uint16 NextFreeIndex = ParticleIndices[ActiveParticles];
            if (!(NextFreeIndex < MaxActiveParticles))
            {
                UE_LOG(LogLevel::Error, TEXT("Detected corrupted particle indices. Template : %s, Component %s"), *(Component && Component ? Component->Template : nullptr)->GetName(), *(Component)->GetName());
                FixupParticleIndices();
                NextFreeIndex = ParticleIndices[ActiveParticles];
            }
            
            DECLARE_PARTICLE_PTR(Particle, ParticleData + ParticleStride * NextFreeIndex);
            const uint32 CurrentParticleIndex = ActiveParticles++;

            PreSpawn(Particle, InitialLocation, InitialVelocity);
            for (int32 ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ++ModuleIndex)
            {
                UParticleModule* SpawnModule = LODLevel->SpawnModules[ModuleIndex];
                // if (SpawnModule->GetFlag(EModuleFlag::Enabled))
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

            SpawnTime -= Increment;
            Interp -= InterpIncrement;
        }
    };

    SpawnInternal();
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
    FPlatformMemory::Memset(Particle, 0,ParticleSize);

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
        // if (LODLevel->EventGenerator)
        // {
        //     EventPayload = (FParticleEventInstancePayload*)GetModuleInstanceData(LODLevel->EventGenerator);
        //     if (EventPayload && (EventPayload->bDeathEventsPresent == false))
        //     {
        //         EventPayload = NULL;
        //     }
        // }

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

bool FParticleEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleEmitterInstance_FillReplayData);

	// NOTE: This the base class implementation that should ONLY be called by derived classes' FillReplayData()!

	// Make sure there is a template present
	if (!SpriteTemplate)
	{
		return false;
	}

	// Allocate it for now, but we will want to change this to do some form
	// of caching
	if (ActiveParticles <= 0 || !bEnabled)
	{
		return false;
	}
	// If the template is disabled, don't return data.
	UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
	if ((LODLevel == NULL) || (LODLevel->bEnabled == false))
	{
		return false;
	}

	// Make sure we will not be allocating enough memory
	assert(MaxActiveParticles >= ActiveParticles);

	// Must be filled in by implementation in derived class
	OutData.eEmitterType = DET_Unknown;

	OutData.ActiveParticleCount = ActiveParticles;
	OutData.ParticleStride = ParticleStride;
	OutData.SortMode = SortMode;

	// Take scale into account
	OutData.Scale = FVector::OneVector;
	if (Component)
	{
		OutData.Scale = FVector(Component->GetComponentTransform().GetScale3D());
	}

	int32 ParticleMemSize = MaxActiveParticles * ParticleStride;

	// Allocate particle memory

	OutData.DataContainer.Alloc(ParticleMemSize, MaxActiveParticles);
	// INC_DWORD_STAT_BY(STAT_RTParticleData, OutData.DataContainer.MemBlockSize);

	FPlatformMemory::Memcpy(OutData.DataContainer.ParticleData, ParticleData, ParticleMemSize);
	FPlatformMemory::Memcpy(OutData.DataContainer.ParticleIndices, ParticleIndices, OutData.DataContainer.ParticleIndicesNumShorts * sizeof(uint16));

	// All particle emitter types derived from sprite emitters, so we can fill that data in here too!
	{
		FDynamicSpriteEmitterReplayDataBase* NewReplayData =
			static_cast< FDynamicSpriteEmitterReplayDataBase* >( &OutData );

		// NewReplayData->RequiredModule = LODLevel->RequiredModule->CreateRendererResource();
		// NewReplayData->MaterialInterface = NULL;	// Must be set by derived implementation
		// NewReplayData->InvDeltaSeconds = (LastDeltaTime > UE_KINDA_SMALL_NUMBER) ? (1.0f / LastDeltaTime) : 0.0f;
		// NewReplayData->LWCTile = ((Component == nullptr) || LODLevel->RequiredModule->bUseLocalSpace) ? FVector::Zero() : Component->GetLWCTile();

		NewReplayData->MaxDrawCount =
		    // (LODLevel->RequiredModule->bUseMaxDrawCount == true) ? LODLevel->RequiredModule->MaxDrawCount :
    	    -1;
		// NewReplayData->ScreenAlignment	= LODLevel->RequiredModule->ScreenAlignment;
		NewReplayData->bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;
		// NewReplayData->EmitterRenderMode = SpriteTemplate->EmitterRenderMode;
		NewReplayData->DynamicParameterDataOffset = DynamicParameterDataOffset;
		NewReplayData->LightDataOffset = LightDataOffset;
		NewReplayData->LightVolumetricScatteringIntensity = LightVolumetricScatteringIntensity;
		NewReplayData->CameraPayloadOffset = CameraPayloadOffset;

		NewReplayData->SubUVDataOffset = SubUVDataOffset;
		NewReplayData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
		NewReplayData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;

		// NewReplayData->MacroUVOverride.bOverride = LODLevel->RequiredModule->bOverrideSystemMacroUV;
		// NewReplayData->MacroUVOverride.Radius = LODLevel->RequiredModule->MacroUVRadius;
		// NewReplayData->MacroUVOverride.Position = FVector3f(LODLevel->RequiredModule->MacroUVPosition);
        
		NewReplayData->bLockAxis = false;
		if (bAxisLockEnabled == true)
		{
			NewReplayData->LockAxisFlag = LockAxisFlags;
			if (LockAxisFlags != EPAL_NONE)
			{
				NewReplayData->bLockAxis = true;
			}
		}

		// If there are orbit modules, add the orbit module data
		// if (LODLevel->OrbitModules.Num() > 0)
		// {
		// 	UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
		// 	UParticleModuleOrbit* LastOrbit = HighestLODLevel->OrbitModules[LODLevel->OrbitModules.Num() - 1];
		// 	assert(LastOrbit);
		//
		// 	uint32* LastOrbitOffset = SpriteTemplate->ModuleOffsetMap.Find(LastOrbit);
		// 	NewReplayData->OrbitModuleOffset = *LastOrbitOffset;
		// }

		// NewReplayData->EmitterNormalsMode = LODLevel->RequiredModule->EmitterNormalsMode;
		// NewReplayData->NormalsSphereCenter = (FVector3f)LODLevel->RequiredModule->NormalsSphereCenter;
		// NewReplayData->NormalsCylinderDirection = (FVector3f)LODLevel->RequiredModule->NormalsCylinderDirection;

    	    
		NewReplayData->PivotOffset = FVector2D(PivotOffset);

		// NewReplayData->bUseVelocityForMotionBlur = LODLevel->RequiredModule->ShouldUseVelocityForMotionBlur();
		// NewReplayData->bRemoveHMDRoll = LODLevel->RequiredModule->bRemoveHMDRoll;
		// NewReplayData->MinFacingCameraBlendDistance = LODLevel->RequiredModule->MinFacingCameraBlendDistance;
		// NewReplayData->MaxFacingCameraBlendDistance = LODLevel->RequiredModule->MaxFacingCameraBlendDistance;       
	}


	return true;
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

void FParticleEmitterInstance::FixupParticleIndices()
{
    // Something is wrong and particle data are be invalid. Try to fix-up things.
    TArray<uint8> UsedIndices;
    UsedIndices.SetNumZeroed(MaxActiveParticles);

    for (int32 i = 0; i < ActiveParticles; ++i)
    {
        const uint16 UsedIndex = ParticleIndices[i];
        if (UsedIndex < MaxActiveParticles && UsedIndices[UsedIndex] == 0)
        {
            UsedIndices[UsedIndex] = 1;
        }
        else
        {
            if (i != ActiveParticles - 1)
            {
                // Remove this bad or duplicated index
                ParticleIndices[i] = ParticleIndices[ActiveParticles - 1];
            }
            // Decrease particle count.
            --ActiveParticles;

            // Retry the new index.
            --i;
        }
    }

    for (int32 i = ActiveParticles; i < MaxActiveParticles; ++i)
    {
        const int32 FreeIndex = UsedIndices.Find(0);
        if ((FreeIndex != INDEX_NONE))
        {
            ParticleIndices[i] =  (uint16)FreeIndex;
        }
        else // Can't really handle that.
        {
            ParticleIndices[i] = (uint16)i;
        }
    }
}

FParticleSpriteEmitterInstance::FParticleSpriteEmitterInstance() : FParticleEmitterInstance()
{
}

FDynamicEmitterDataBase* FParticleSpriteEmitterInstance::GetDynamicData(bool bSelected)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleSpriteEmitterInstance_GetDynamicData);

    // It is valid for the LOD level to be NULL here!
    UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    if (
        // IsDynamicDataRequired(LODLevel) == false ||
        !bEnabled)
    {
        return NULL;
    }

    // Allocate the dynamic data
    FDynamicSpriteEmitterData* NewEmitterData = new FDynamicSpriteEmitterData(LODLevel->RequiredModule);
    {
        // INC_DWORD_STAT(STAT_DynamicEmitterCount);
        // INC_DWORD_STAT(STAT_DynamicSpriteCount);
        // INC_DWORD_STAT_BY(STAT_DynamicEmitterMem, sizeof(FDynamicSpriteEmitterData));
    }

    // Now fill in the source data
    if( !FillReplayData( NewEmitterData->Source ) )
    {
        delete NewEmitterData;
        return NULL;
    }

    // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
    NewEmitterData->Init( bSelected );

    return NewEmitterData;
}

bool FParticleSpriteEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleSpriteEmitterInstance_FillReplayData);

    if (ActiveParticles <= 0)
    {
        return false;
    }

    // Call parent implementation first to fill in common particle source data
    if( !FParticleEmitterInstance::FillReplayData( OutData ) )
    {
        return false;
    }

    OutData.eEmitterType = DET_Sprite;

    FDynamicSpriteEmitterReplayData* NewReplayData =
        static_cast< FDynamicSpriteEmitterReplayData* >( &OutData );

    // Get the material instance. If there is none, or the material isn't flagged for use with particle systems, use the DefaultMaterial.
    // NewReplayData->MaterialInterface = GetCurrentMaterial();

    return true;
}

FParticleMeshEmitterInstance::FParticleMeshEmitterInstance() :
      FParticleEmitterInstance()
    , MeshTypeData(NULL)
    , MeshRotationActive(false)
    , MeshRotationOffset(0)
    , MeshMotionBlurOffset(0)
{
}


void FParticleMeshEmitterInstance::InitParameters(UParticleEmitter* InTemplate, UParticleSystemComponent* InComponent)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_MeshEmitterInstance_InitParameters);

    FParticleEmitterInstance::InitParameters(InTemplate, InComponent);

    // Get the type data module
    UParticleLODLevel* LODLevel	= InTemplate->GetLODLevel(0);
    assert(LODLevel);
    MeshTypeData = CastChecked<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
    assert(MeshTypeData);

    // TODO
    // Grab cached mesh rotation flag from ParticleEmitter template
    // MeshRotationActive = InTemplate->bMeshRotationActive;
}

bool FParticleMeshEmitterInstance::Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount)
{
    int32 OldMaxActiveParticles = MaxActiveParticles;
    if (FParticleEmitterInstance::Resize(NewMaxActiveParticles, bSetMaxActiveCount) == true)
    {
        if (MeshRotationActive)
        {
            for (int32 i = OldMaxActiveParticles; i < NewMaxActiveParticles; i++)
            {
                DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
                // FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
                // PayloadData->RotationRateBase			= FVector3f::ZeroVector;
            }
        }
		
        return true;
    }

    return false;}

void FParticleMeshEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_MeshTickTime);

    if (bEnabled && MeshMotionBlurOffset)
    {
        for (int32 i = 0; i < ActiveParticles; i++)
        {
            DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);

            // FMeshRotationPayloadData* RotationPayloadData = (FMeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
            // FMeshMotionBlurPayloadData* MotionBlurPayloadData = (FMeshMotionBlurPayloadData*)((uint8*)&Particle + MeshMotionBlurOffset);

            // MotionBlurPayloadData->BaseParticlePrevRotation = Particle.Rotation;
            // MotionBlurPayloadData->BaseParticlePrevVelocity = Particle.Velocity;
            // MotionBlurPayloadData->BaseParticlePrevSize = Particle.Size;
            // MotionBlurPayloadData->PayloadPrevRotation = RotationPayloadData->Rotation;
            //
            // if (CameraPayloadOffset)
            // {
            //     const FCameraOffsetParticlePayload* CameraPayload = (const FCameraOffsetParticlePayload*)((const uint8*)&Particle + CameraPayloadOffset);
            //     MotionBlurPayloadData->PayloadPrevCameraOffset = CameraPayload->Offset;
            // }
            // else
            // {
            //     MotionBlurPayloadData->PayloadPrevCameraOffset = 0.0f;
            // }
            //
            // if (OrbitModuleOffset)
            // {
            //     const FOrbitChainModuleInstancePayload* OrbitPayload = (const FOrbitChainModuleInstancePayload*)((const uint8*)&Particle + OrbitModuleOffset);
            //     MotionBlurPayloadData->PayloadPrevOrbitOffset = OrbitPayload->Offset;
            // }
            // else
            // {
            //     MotionBlurPayloadData->PayloadPrevOrbitOffset = FVector3f::ZeroVector;
            // }
        }
    }

    UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    // See if we are handling mesh rotation
    if (MeshRotationActive && bEnabled)
    {
        // Update the rotation for each particle
        for (int32 i = 0; i < ActiveParticles; i++)
        {
            // DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            // FMeshRotationPayloadData* PayloadData	= (FMeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
            // PayloadData->RotationRate				= PayloadData->RotationRateBase;
            // if (LODLevel->RequiredModule->ScreenAlignment == PSA_Velocity
            //     || LODLevel->RequiredModule->ScreenAlignment == PSA_AwayFromCenter)
            // {
            //     // Determine the rotation to the velocity vector and apply it to the mesh
            //     FVector	NewDirection	= (FVector)Particle.Velocity;
				        //
            //     if (LODLevel->RequiredModule->ScreenAlignment == PSA_Velocity)
            //     {
            //         //check if an orbit module should affect the velocity...		
            //         if (LODLevel->RequiredModule->bOrbitModuleAffectsVelocityAlignment &&
            //             LODLevel->OrbitModules.Num() > 0)
            //         {
            //             UParticleModuleOrbit* LastOrbit = SpriteTemplate->LODLevels[0]->OrbitModules[LODLevel->OrbitModules.Num() - 1];
            //             check(LastOrbit);
					       //
            //             uint32 SpriteOrbitModuleOffset = *SpriteTemplate->ModuleOffsetMap.Find(LastOrbit);
            //             if (SpriteOrbitModuleOffset != 0)
            //             {
            //                 const FOrbitChainModuleInstancePayload &OrbitPayload = *(FOrbitChainModuleInstancePayload*)((uint8*)&Particle + SpriteOrbitModuleOffset);
            //
            //                 //this should be our current position
            //                 const FVector NewPos = Particle.Location + (FVector)OrbitPayload.Offset;
            //                 //this should be our previous position
            //                 const FVector OldPos = Particle.OldLocation + (FVector)OrbitPayload.PreviousOffset;
            //
            //                 NewDirection = NewPos - OldPos;
            //             }
            //         }
            //     }
            //     else if (LODLevel->RequiredModule->ScreenAlignment == PSA_AwayFromCenter)
            //     {
            //         NewDirection = Particle.Location;
            //     }
            //
            //     NewDirection.Normalize();
            //     FVector	OldDirection(1.0f, 0.0f, 0.0f);
            //
            //     FQuat Rotation = FQuat::FindBetweenNormals(OldDirection, NewDirection);
            //     FVector3f Euler(Rotation.Euler());
            //     PayloadData->Rotation = PayloadData->InitRotation + Euler;
            //     PayloadData->Rotation += PayloadData->CurContinuousRotation;
            // }
            // else // not PSA_Velocity or PSA_AwayfromCenter, so rotation is not reset every tick
            // {
            //     if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
            //     {
            //         PayloadData->Rotation = PayloadData->InitRotation + PayloadData->CurContinuousRotation;
            //     }
            // }
        }
    }


    // Call the standard tick
    FParticleEmitterInstance::Tick(DeltaTime, bSuppressSpawning);

    if (MeshRotationActive && bEnabled)
    {
        //Must do this (at least) after module update other wise the reset value of RotationRate is used.
        //Probably the other stuff before the module tick should be brought down here too and just leave the RotationRate reset before.
        //Though for the sake of not breaking existing behavior, leave things as they are for now.
        for (int32 i = 0; i < ActiveParticles; i++)
        {
            DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
            // FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
            // PayloadData->CurContinuousRotation += DeltaTime * PayloadData->RotationRate;
        }
    }

    // Remove from the Sprite count... happens because we use the Super::Tick
    // DEC_DWORD_STAT_BY(STAT_SpriteParticles, ActiveParticles);
    // INC_DWORD_STAT_BY(STAT_MeshParticles, ActiveParticles);
}

void FParticleMeshEmitterInstance::UpdateBoundingBox(float DeltaTime)
{
	// SCOPE_CYCLE_COUNTER(STAT_ParticleUpdateBounds);
	//@todo. Implement proper bound determination for mesh emitters.
	// Currently, just 'forcing' the mesh size to be taken into account.
	if ((Component != NULL) && (ActiveParticles > 0))
	{
		bool bUpdateBox =
		    // ((Component->bWarmingUp == false) &&
			(Component->Template != NULL)
			// && (Component->Template->bUseFixedRelativeBoundingBox == false))
	    ;

		// Take scale into account
		FVector Scale = Component->GetComponentTransform().GetScale3D();

		// Get the static mesh bounds
		// FBoxSphereBounds MeshBound;
		// if (Component->bWarmingUp == false)
		// {	
		// 	if (MeshTypeData->Mesh)
		// 	{
		// 		MeshBound = MeshTypeData->Mesh->GetBounds();
		// 	}
		// 	else
		// 	{
		// 		//UE_LOG(LogParticles, Log, TEXT("MeshEmitter with no mesh set?? - %s"), Component->Template ? *(Component->Template->GetPathName()) : TEXT("??????"));
		// 		MeshBound = FBoxSphereBounds(FVector(0, 0, 0), FVector(0, 0, 0), 0);
		// 	}
		// }
		// else
		// {
		// 	// This isn't used anywhere if the bWarmingUp flag is false, but GCC doesn't like it not touched.
		// 	FMemory::Memzero(&MeshBound, sizeof(FBoxSphereBounds));
		// }

		UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

		const bool bUseLocalSpace = LODLevel->RequiredModule->bUseLocalSpace;

		const FMatrix ComponentToWorld = bUseLocalSpace 
			? Component->GetWorldMatrix() 
			: FMatrix::Identity;

		FVector	NewLocation;
		float	NewRotation;
		if (bUpdateBox)
		{
			// ParticleBoundingBox.Init();
		}

		// For each particle, offset the box appropriately 
		// FVector MinVal(HALF_WORLD_MAX);
		// FVector MaxVal(-HALF_WORLD_MAX);
		
		//FPlatformMisc::Prefetch(ParticleData, ParticleStride * ParticleIndices[0]);
		//FPlatformMisc::Prefetch(ParticleData, (ParticleIndices[0] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);

		// bool bSkipDoubleSpawnUpdate = !SpriteTemplate->bUseLegacySpawningBehavior;
		for (int32 i=0; i<ActiveParticles; i++)
		{
			DECLARE_PARTICLE(Particle, ParticleData + ParticleStride * ParticleIndices[i]);
			// FPlatformMisc::Prefetch(ParticleData, ParticleStride * ParticleIndices[i+1]);
			// FPlatformMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);

			// Do linear integrator and update bounding box
			Particle.OldLocation = Particle.Location;

			bool bJustSpawned = (Particle.Flags & STATE_Particle_JustSpawned) != 0;
			Particle.Flags &= ~STATE_Particle_JustSpawned;

			//Don't update position for newly spawned particles. They already have a partial update applied during spawn.
			bool bSkipUpdate = bJustSpawned
		    // && bSkipDoubleSpawnUpdate
		    ;

			if ((Particle.Flags & STATE_Particle_Freeze) == 0 && !bSkipUpdate)
			{
				if ((Particle.Flags & STATE_Particle_FreezeTranslation) == 0)
				{
					NewLocation	= Particle.Location + (FVector)Particle.Velocity * DeltaTime;
				}
				else
				{
					NewLocation = Particle.Location;
				}
				if ((Particle.Flags & STATE_Particle_FreezeRotation) == 0)
				{
					NewRotation	= Particle.Rotation + DeltaTime * Particle.RotationRate;
				}
				else
				{
					NewRotation = Particle.Rotation;
				}
			}
			else
			{
				// Don't move it...
				NewLocation = Particle.Location;
				NewRotation = Particle.Rotation;
			}

			// FVector LocalExtent = MeshBound.GetBox().GetExtent() * (FVector)Particle.Size * Scale;

			// NewLocation			+= PositionOffsetThisTick;
			// Particle.OldLocation+= PositionOffsetThisTick;

			// Do angular integrator, and wrap result to within +/- 2 PI
			Particle.Rotation = FMath::Fmod(NewRotation, 2.f * (float)PI);
			Particle.Location = NewLocation;

			if (bUpdateBox)
			{	
				FVector PositionForBounds = NewLocation;

				if (bUseLocalSpace)
				{
					// Note: building the bounding box in world space as that gives tighter bounds than transforming a local space AABB into world space
					PositionForBounds = ComponentToWorld.TransformPosition(NewLocation);
				}

				// MinVal[0] = FMath::Min(MinVal[0], PositionForBounds.X - LocalExtent.X);
				// MaxVal[0] = FMath::Max(MaxVal[0], PositionForBounds.X + LocalExtent.X);
				// MinVal[1] = FMath::Min(MinVal[1], PositionForBounds.Y - LocalExtent.Y);
				// MaxVal[1] = FMath::Max(MaxVal[1], PositionForBounds.Y + LocalExtent.Y);
				// MinVal[2] = FMath::Min(MinVal[2], PositionForBounds.Z - LocalExtent.Z);
				// MaxVal[2] = FMath::Max(MaxVal[2], PositionForBounds.Z + LocalExtent.Z);
			}
		}

		// if (bUpdateBox)
		// {	
		// 	ParticleBoundingBox = FBox(MinVal, MaxVal);
		// }
	}
}

void FParticleMeshEmitterInstance::PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime)
{
    FParticleEmitterInstance::PostSpawn(Particle, InterpolationPercentage, SpawnTime);
	UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();

	// FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((uint8*)Particle + MeshRotationOffset);

	// if (LODLevel->RequiredModule->ScreenAlignment == PSA_Velocity
	// 	|| LODLevel->RequiredModule->ScreenAlignment == PSA_AwayFromCenter)
	// {
	// 	// Determine the rotation to the velocity vector and apply it to the mesh
	// 	FVector	NewDirection(Particle->Velocity);
	// 	if (LODLevel->RequiredModule->ScreenAlignment == PSA_AwayFromCenter)
	// 	{
	// 		NewDirection = Particle->Location;
	// 	}
	//
	// 	NewDirection.Normalize();
	// 	FVector	OldDirection(1.0f, 0.0f, 0.0f);
	//
	// 	FQuat Rotation	= FQuat::FindBetweenNormals(OldDirection, NewDirection);
	// 	FVector Euler	= Rotation.Euler();
	//
	// 	PayloadData->Rotation.X	+= Euler.X;
	// 	PayloadData->Rotation.Y	+= Euler.Y;
	// 	PayloadData->Rotation.Z	+= Euler.Z;
	// }

	// FVector InitialOrient = MeshTypeData->RollPitchYawRange.GetValue(SpawnTime, 0, 0, &MeshTypeData->RandomStream);
	// PayloadData->InitialOrientation = (FVector3f)InitialOrient;

	// if (MeshMotionBlurOffset)
	// {
	// 	FMeshRotationPayloadData* RotationPayloadData = (FMeshRotationPayloadData*)((uint8*)Particle + MeshRotationOffset);
	// 	FMeshMotionBlurPayloadData* MotionBlurPayloadData = (FMeshMotionBlurPayloadData*)((uint8*)Particle + MeshMotionBlurOffset);
	//
	// 	MotionBlurPayloadData->BaseParticlePrevRotation = Particle->Rotation;
	// 	MotionBlurPayloadData->BaseParticlePrevVelocity = Particle->Velocity;
	// 	MotionBlurPayloadData->BaseParticlePrevSize = Particle->Size;
	// 	MotionBlurPayloadData->PayloadPrevRotation = RotationPayloadData->Rotation;
	//
	// 	if (CameraPayloadOffset)
	// 	{
	// 		const FCameraOffsetParticlePayload* CameraPayload = (const FCameraOffsetParticlePayload*)((const uint8*)Particle + CameraPayloadOffset);
	// 		MotionBlurPayloadData->PayloadPrevCameraOffset = CameraPayload->Offset;
	// 	}
	// 	else
	// 	{
	// 		MotionBlurPayloadData->PayloadPrevCameraOffset = 0.0f;
	// 	}
	//
	// 	if (OrbitModuleOffset)
	// 	{
	// 		const FOrbitChainModuleInstancePayload* OrbitPayload = (const FOrbitChainModuleInstancePayload*)((const uint8*)Particle + OrbitModuleOffset);
	// 		MotionBlurPayloadData->PayloadPrevOrbitOffset = OrbitPayload->Offset;
	// 	}
	// 	else
	// 	{
	// 		MotionBlurPayloadData->PayloadPrevOrbitOffset = FVector3f::ZeroVector;
	// 	}
	// }
}

FDynamicEmitterDataBase* FParticleMeshEmitterInstance::GetDynamicData(bool bSelected)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_ParticleMeshEmitterInstance_GetDynamicData);

    // It is safe for LOD level to be NULL here!
    UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    if (
        // IsDynamicDataRequired(LODLevel) == false ||
        !bEnabled)
    {
        return NULL;
    }

    // Allocate the dynamic data
    FDynamicMeshEmitterData* NewEmitterData = new FDynamicMeshEmitterData(LODLevel->RequiredModule);
    {
        // INC_DWORD_STAT(STAT_DynamicEmitterCount);
        // INC_DWORD_STAT(STAT_DynamicMeshCount);
        // INC_DWORD_STAT_BY(STAT_DynamicEmitterMem, sizeof(FDynamicMeshEmitterData));
    }

    // Now fill in the source data
    if( !FillReplayData( NewEmitterData->Source ) )
    {
        delete NewEmitterData;
        return NULL;
    }


    // Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
    NewEmitterData->Init(
        this,
        MeshTypeData->Mesh,
        MeshTypeData->LODSizeScale
        );

    return NewEmitterData;
}

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
	
	// Mesh settings
	// NewReplayData->bScaleUV = LODLevel->RequiredModule->bScaleUV;
	// NewReplayData->SubUVInterpMethod = LODLevel->RequiredModule->InterpolationMethod;
	NewReplayData->SubUVDataOffset = SubUVDataOffset;
	NewReplayData->SubImages_Horizontal = LODLevel->RequiredModule->SubImages_Horizontal;
	NewReplayData->SubImages_Vertical = LODLevel->RequiredModule->SubImages_Vertical;
	// NewReplayData->MeshRotationOffset = MeshRotationOffset;
	// NewReplayData->MeshMotionBlurOffset = MeshMotionBlurOffset;
	// NewReplayData->bMeshRotationActive = MeshRotationActive;
	// NewReplayData->MeshAlignment = MeshTypeData->MeshAlignment;
	
	// Scale needs to be handled in a special way for meshes.  The parent implementation set this
	// itself, but we'll recompute it here.
	NewReplayData->Scale = FVector::OneVector;
	if (Component)
	{
		assert(SpriteTemplate);
		UParticleLODLevel* LODLevel2 = SpriteTemplate->GetCurrentLODLevel(this);
		assert(LODLevel2);
		assert(LODLevel2->RequiredModule);
		// Take scale into account
		if (LODLevel2->RequiredModule->bUseLocalSpace == false)
		{
			// if (!bIgnoreComponentScale)
			{
				NewReplayData->Scale = Component->GetWorldScale3D();
			}
		}
	}
	
	// See if the new mesh locked axis is being used...
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
    if (LODLevel->RequiredModule->bUseLocalSpace == false)
    {
        if (FVector::Distance(OldLocation, Location) > 1.f)
        {
            Particle->Location += (OldLocation - Location) * InterpolationPercentage;	
        }
    }
    
    // Offset caused by any velocity
    Particle->OldLocation = Particle->Location;
    Particle->Location   += FVector(Particle->Velocity) * SpawnTime;
    
    // Store a sequence counter.
    Particle->Flags |= ((ParticleCounter++) & STATE_CounterMask);
    Particle->Flags |= STATE_Particle_JustSpawned;
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
        //EmitterDurations.InsertUninitialized(0, SpriteTemplate->LODLevels.Num());
        // 만약 퍼포먼스적으로 문제가 발생할 경우 아래 코드를 우선적으로 수정할 것.
        EmitterDurations.AddUninitialized(SpriteTemplate->LODLevels.Num());
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

FParticleEmitterInstance::FParticleEmitterInstance() :
      SpriteTemplate(NULL)
    , Component(NULL)
    , CurrentLODLevel(NULL)
    , CurrentLODLevelIndex(0)
    , TypeDataOffset(0)
    , TypeDataInstanceOffset(-1)
    , SubUVDataOffset(0)
    , DynamicParameterDataOffset(0)
    , LightDataOffset(0)
    , LightVolumetricScatteringIntensity(0)
    // , OrbitModuleOffset(0)
    , CameraPayloadOffset(0)
    , PayloadOffset(0)
    , bEnabled(1)
    , bKillOnDeactivate(0)
    , bKillOnCompleted(0)
    , bHaltSpawning(0)
    , bHaltSpawningExternal(0)
    , bRequiresLoopNotification(0)
    // , bIgnoreComponentScale(0)
    // , bIsBeam(0)
    , bAxisLockEnabled(0)
    // , bFakeBurstsWhenSpawningSupressed(0)
    , LockAxisFlags(EPAL_NONE)
    , SortMode(PSORTMODE_None)
    , ParticleData(NULL)
    , ParticleIndices(NULL)
    , InstanceData(NULL)
    , InstancePayloadSize(0)
    , ParticleSize(0)
    , ParticleStride(0)
    , ActiveParticles(0)
    , ParticleCounter(0)
    , MaxActiveParticles(0)
    , SpawnFraction(0.0f)
    , SecondsSinceCreation(0.0f)
    , EmitterTime(0.0f)
    , LoopCount(0)
    , IsRenderDataDirty(0)
    , EmitterDuration(0.0f)
    , TrianglesToRender(0)
    , MaxVertexIndex(0)
    , CurrentMaterial(NULL)
    // #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    // , EventCount(0)
    // , MaxEventCount(0)
    // #endif	//#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    // , PositionOffsetThisTick(0)
    , PivotOffset(-0.5f,-0.5f)
{
}

FParticleEmitterInstance::~FParticleEmitterInstance()
{
    // for (int32 i = 0; i < HighQualityLights.Num(); ++i)
    // {
    //     UPointLightComponent* PointLightComponent = HighQualityLights[i];
    //     {
    //         PointLightComponent->Modify();
    //         PointLightComponent->DestroyComponent(false);
    //     }
    // }
    // HighQualityLights.Reset();

    FPlatformMemory::Free<EAT_Container>(ParticleData, ParticleStride * MaxActiveParticles);
    FPlatformMemory::Free<EAT_Container>(ParticleIndices, sizeof(uint16) * (MaxActiveParticles + 1));
    // FPlatformMemory::Free(InstanceData);
    // BurstFired.Empty();
}

void FParticleEmitterInstance::SetMeshMaterials(const TArray<UMaterial*>& InMaterials)
{
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
        // LockAxisFlags = SpriteTemplate->LockAxisFlags;
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
            InstanceData = (uint8*)(FPlatformMemory::Realloc<EAT_Object>(InstanceData, SpriteTemplate->ReqInstanceBytes));
            InstancePayloadSize = SpriteTemplate->ReqInstanceBytes;
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
    // SetMeshMaterials(SpriteTemplate->MeshMaterials);

    // Set initial values.
    SpawnFraction = 0;
    SecondsSinceCreation = 0;
    EmitterTime = 0;
    ParticleCounter = 0;

    UpdateTransforms();

    // Begin Test
    //Location = Component->GetComponentLocation();
    Location = Component->GetWorldLocation(); // Location=Component->GetRelativeLocation();
    // End Test

    OldLocation = Location;

    TrianglesToRender = 0;
    MaxVertexIndex = 0;

    if (ParticleData == NULL)
    {
        MaxActiveParticles = 0;
        ActiveParticles = 0;
    }

    // ParticleBoundingBox.Init();
    if (HighLODLevel->RequiredModule->RandomImageChanges == 0)
    {
        HighLODLevel->RequiredModule->RandomImageTime = 1.0f;
    }
    else
    {
        HighLODLevel->RequiredModule->RandomImageTime = 0.99f / (HighLODLevel->RequiredModule->RandomImageChanges + 1);
    }

    //if (bNeedsInit &&
    //    Component->GetWorld()->IsGameWorld() == true &&
    //    // Only presize if any particles will be spawned 
    //    SpriteTemplate->QualityLevelSpawnRateScale > 0)
    if (bNeedsInit && SpriteTemplate->QualityLevelSpawnRateScale > 0) // Resize to sensible default.
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
        // if (BurstFired.Num() < SpriteTemplate->LODLevels.Num())
        // {
        //     BurstFired.AddZeroed(SpriteTemplate->LODLevels.Num() - BurstFired.Num());
        // }

        // for (int32 LODIndex = 0; LODIndex < SpriteTemplate->LODLevels.Num(); LODIndex++)
        // {
        //     UParticleLODLevel* LODLevel = SpriteTemplate->LODLevels[LODIndex];
        //     assert(LODLevel);
        //     FLODBurstFired& LocalBurstFired = BurstFired[LODIndex];
        //     if (LocalBurstFired.Fired.Num() < LODLevel->SpawnModule->BurstList.Num())
        //     {
        //         LocalBurstFired.Fired.AddZeroed(LODLevel->SpawnModule->BurstList.Num() - LocalBurstFired.Fired.Num());
        //     }
        // }
    }

    // ResetBurstList();

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
    // IsRenderDataDirty = 1;
    //
    // bEmitterIsDone = false;
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

    // if (bHasSubUV)
    // {
    //     SubUVDataOffset = PayloadOffset;
    //     uiBytes = sizeof(FFullSubUVPayload);
    // }

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


void FParticleEmitterInstance::UpdateTransforms()
{
    //QUICK_SCOPE_CYCLE_COUNTER(STAT_EmitterInstance_UpdateTransforms);

    assert(SpriteTemplate != NULL);

    UParticleLODLevel* LODLevel = GetCurrentLODLevelChecked();
    FMatrix ComponentToWorld = Component != NULL ?
        Component->GetComponentTransform().ToMatrixNoScale() : FMatrix::Identity;
    FMatrix EmitterToComponent = FMatrix::CreateRotationTranslationMatrix(
        LODLevel->RequiredModule->EmitterRotation,
        LODLevel->RequiredModule->EmitterOrigin
    );
    if (LODLevel->RequiredModule->bUseLocalSpace)
    {
        EmitterToSimulation = EmitterToComponent;
        SimulationToWorld = ComponentToWorld;
#if ENABLE_NAN_DIAGNOSTIC
        if (SimulationToWorld.ContainsNaN())
        {
            logOrEnsureNanError(TEXT("FParticleEmitterInstance::UpdateTransforms() - SimulationToWorld contains NaN!"));
            SimulationToWorld = FMatrix::Identity;
        }
#endif
    }
    else
    {
        EmitterToSimulation = EmitterToComponent * ComponentToWorld;
        SimulationToWorld = FMatrix::Identity;
    }
}

/**
 *	Reset the burst list information for the instance
 */
void FParticleEmitterInstance::ResetBurstList()
{
    //QUICK_SCOPE_CYCLE_COUNTER(STAT_ResetBurstLists);

    // for (int32 BurstIndex = 0; BurstIndex < BurstFired.Num(); BurstIndex++)
    // {
    //     FLODBurstFired& CurrBurstFired = BurstFired[BurstIndex];
    //     for (int32 FiredIndex = 0; FiredIndex < CurrBurstFired.Fired.Num(); FiredIndex++)
    //     {
    //         CurrBurstFired.Fired[FiredIndex] = false;
    //     }
    // }
}
