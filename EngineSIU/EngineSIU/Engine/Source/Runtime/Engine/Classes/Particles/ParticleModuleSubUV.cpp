#include "ParticleModuleSubUV.h"

#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "TypeData/ParticleModuleTypeDataBase.h"

UParticleModuleSubUV::UParticleModuleSubUV()
{
    Flags = static_cast<EModuleFlag::EModuleFlags>(EModuleFlag::SpawnModule | EModuleFlag::UpdateModule);
}

void UParticleModuleSubUV::Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);
    EParticleSubUVInterpMethod InterpMethod = LODLevel->RequiredModule->InterpolationMethod;
    const uint32 PayloadOffset = Owner->SubUVDataOffset;
    if (InterpMethod == PSUVIM_None || PayloadOffset == 0)
        return;
    
    // TypeDataModule이 없을 때 or TypeDataModule이 있으면서 SubUV를 지원할 때
    if (!LODLevel->TypeDataModule || LODLevel->TypeDataModule->SupportsSubUV())
    {
        SPAWN_INIT;
        {
            uint32 TempOffset = CurrentOffset;
            CurrentOffset = PayloadOffset;
            PARTICLE_ELEMENT(FFullSubUVPayload, SubUVPayload)
            CurrentOffset = TempOffset;

            SubUVPayload.ImageIndex = DetermineImageIndex(Owner, Offset, &Particle, InterpMethod, SubUVPayload, SpawnTime);
        }
    }
}

void UParticleModuleSubUV::Update(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime)
{
    UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);
    EParticleSubUVInterpMethod InterpMethod = LODLevel->RequiredModule->InterpolationMethod;
    const int32 PayloadOffset = Owner->SubUVDataOffset;
    if (InterpMethod == PSUVIM_None || PayloadOffset == 0)
        return;
    if (InterpMethod == PSUVIM_Random || InterpMethod == PSUVIM_Random_Blend)
    {
        if (LODLevel->RequiredModule->RandomImageChanges == 0)
            return;
    }
    
    if (!LODLevel->TypeDataModule || LODLevel->TypeDataModule->SupportsSubUV())
    {
        BEGIN_UPDATE_LOOP;
            if (Particle.RelativeTime > 1.0f)
            {
                CONTINUE_UPDATE_LOOP;
            }
            uint32 TempOffset = CurrentOffset;
            CurrentOffset = PayloadOffset;
            PARTICLE_ELEMENT(FFullSubUVPayload, SubUVPayload);
            CurrentOffset = TempOffset;

            SubUVPayload.ImageIndex = DetermineImageIndex(Owner, Offset, &Particle, InterpMethod, SubUVPayload, DeltaTime);
        END_UPDATE_LOOP;
    }
}

float UParticleModuleSubUV::DetermineImageIndex(FParticleEmitterInstance* Owner, int32 Offset, FBaseParticle* Particle,
    EParticleSubUVInterpMethod InterpMethod, FFullSubUVPayload& SubUVPayload, float DeltaTime)
{
    UParticleLODLevel* LODLevel = Owner->SpriteTemplate->GetCurrentLODLevel(Owner);
    const int32 TotalSubImages = LODLevel->RequiredModule->SubImages_Horizontal * LODLevel->RequiredModule->SubImages_Vertical;
    float ImageIndex = SubUVPayload.ImageIndex;

    if (InterpMethod == PSUVIM_Linear || InterpMethod == PSUVIM_Linear_Blend)
    {
        ImageIndex = SubImageIndex.GetValue(Particle->RelativeTime);
        if (InterpMethod == PSUVIM_Linear)
        {
            ImageIndex = FMath::FloorToInt(ImageIndex);
        }
    } else if (InterpMethod == PSUVIM_Random || InterpMethod == PSUVIM_Random_Blend)
    {
        if ((LODLevel->RequiredModule->RandomImageChanges == 0.f) ||
            (Particle->RelativeTime - SubUVPayload.RandomImageTime > LODLevel->RequiredModule->RandomImageChanges) ||
            (SubUVPayload.RandomImageTime == 0.f))
        {
            ImageIndex = GetRandomStream(Owner).RandHelper(TotalSubImages);
            SubUVPayload.RandomImageTime = Particle->RelativeTime;
        }
        if (InterpMethod == PSUVIM_Random)
        {
            ImageIndex = FMath::FloorToInt(ImageIndex);
        }
    } else
    {
        ImageIndex = 0;
    }
    return ImageIndex;
}

void UParticleModuleSubUV::SetToSensibleDefaults(UParticleEmitter* Owner)
{
    // SubImageIndex.Distribution = NewObject<UDistributionFloatConstantCurve>(this);
    // UDistributionFloatConstantCurve* SubImageIndexDist = Cast<UDistributionFloatConstantCurve>(SubImageIndex.Distribution);
    // if (SubImageIndexDist)
    // {
    //     // Add two points, one at time 0.0f and one at 1.0f
    //     for (int32 Key = 0; Key < 2; Key++)
    //     {
    //         int32	KeyIndex = SubImageIndexDist->CreateNewKey(Key * 1.0f);
    //         SubImageIndexDist->SetKeyOut(0, KeyIndex, 0.0f);
    //     }
    //     SubImageIndexDist->bIsDirty = true;
    // }
}


