#include "ParticleModule.h"
#include "ParticleHelper.h"
#include "ParticleEmitterInstances.h"
#include "ParticleModuleLifetime.h"
#include "ParticleSystemComponent.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"

FRandomStream& UParticleModule::GetRandomStream(FParticleEmitterInstance* Owner)
{
    FParticleRandomSeedInstancePayload* Payload = Owner->GetModuleRandomSeedInstanceData(this);
    FRandomStream& RandomStream = (Payload != nullptr) ? Payload->RandomStream : Owner->Component->RandomStream;
    return RandomStream;
}

uint32 UParticleModule::PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData)
{
    return 0xffffffff;
}

FParticleRandomSeedInfo* UParticleModule::GetRandomSeedInfo()
{
    return NULL;
}

uint32 UParticleModule::PrepRandomSeedInstancePayload(FParticleEmitterInstance* Owner, FParticleRandomSeedInstancePayload* InRandSeedPayload, const FParticleRandomSeedInfo& InRandSeedInfo)
{
    assert(Owner != nullptr && "Owner should not be nullptr");
    assert(Owner->Component != nullptr && "Owner->Component should not be nullptr");
    if (!(Owner != nullptr && Owner->Component != nullptr))
    {
        return 0xffffffff;
    }


    if (InRandSeedPayload != nullptr)
    {
        new(InRandSeedPayload) FParticleRandomSeedInstancePayload();

        // See if the parameter is set on the instance...
        // if (InRandSeedInfo.bGetSeedFromInstance == true)
        // {
        //     float SeedValue;
        //     if (Owner->Component->GetFloatParameter(InRandSeedInfo.ParameterName, SeedValue) == true)
        //     {
        //         if (InRandSeedInfo.bInstanceSeedIsIndex == false)
        //         {
        //             InRandSeedPayload->RandomStream.Initialize(static_cast<int32>(std::round(SeedValue)));
        //         }
        //         else
        //         {
        //             if (InRandSeedInfo.RandomSeeds.Num() > 0)
        //             {
        //                 int32 Index = FMath::Min<int32>((InRandSeedInfo.RandomSeeds.Num() - 1), static_cast<int32>(std::trunc(SeedValue)));
        //                 InRandSeedPayload->RandomStream.Initialize(InRandSeedInfo.RandomSeeds[Index]);
        //                 return 0;
        //             }
        //             else
        //             {
        //                 return 0xffffffff;
        //             }
        //         }
        //         return 0;
        //     }
        // }

        // Pick a seed to use and initialize it!!!!
        if (InRandSeedInfo.RandomSeeds.Num() > 0)
        {
            int32 Index = 0;

            if (InRandSeedInfo.bRandomlySelectSeedArray)
            {
                Index = Owner->Component->RandomStream.RandHelper(InRandSeedInfo.RandomSeeds.Num());
            }

            InRandSeedPayload->RandomStream.Initialize(InRandSeedInfo.RandomSeeds[Index]);
            return 0;
        }
        // FIX-ME
        //else if (FApp::bUseFixedSeed)
        else
        {
            InRandSeedPayload->RandomStream.Initialize(GetFName());
            return 0;
        }
    }
    return 0xffffffff;
}

void UParticleModule::Spawn(FParticleEmitterInstance* Owner, uint32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
}

void UParticleModule::Update(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime)
{
}

uint32 UParticleModule::RequiredBytes(UParticleModuleTypeDataBase* TypeData)
{
    return 0;
}

uint32 UParticleModule::RequiredBytesPerInstance()
{
    return 0;
}

void UParticleModule::SetToSensibleDefaults(UParticleEmitter* Owner)
{
}

void UParticleModule::FinalUpdate(FParticleEmitterInstance* Owner, uint32 Offset, float DeltaTime)
{
}

UParticleModuleTypeDataBase::UParticleModuleTypeDataBase() : Super()
{
    // bSpawnModule = false;
    // bUpdateModule = false;
}

FParticleEmitterInstance* UParticleModuleTypeDataBase::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
    return NULL;
}

UParticleModuleTypeDataMesh::UParticleModuleTypeDataMesh() : Super()
{
    Mesh = nullptr;
    CastShadows = false;
    DoCollisions = false;
    // MeshAlignment = PSMA_MeshFaceCameraWithRoll;
    // AxisLockOption = EPAL_NONE;
    // CameraFacingUpAxisOption_DEPRECATED = CameraFacing_NoneUP;
    // CameraFacingOption = XAxisFacing_NoUp;
    // bCollisionsConsiderPartilceSize = true;
    bUseStaticMeshLODs = true;
    LODSizeScale = 1.0f;
}

void UParticleModuleTypeDataMesh::SetToSensibleDefaults(UParticleEmitter* Owner)
{
    if ((Mesh == NULL))
    {
        FResourceManager::CreateStaticMesh("Contents/Reference/Reference.obj");
        Mesh = FResourceManager::GetStaticMesh(L"Contents/Reference/Reference.obj");
    }
}

FParticleEmitterInstance* UParticleModuleTypeDataMesh::CreateInstance(UParticleEmitter* InEmitterParent, UParticleSystemComponent* InComponent)
{
    SetToSensibleDefaults(InEmitterParent);
    FParticleEmitterInstance* Instance = new FParticleMeshEmitterInstance();
    assert(Instance);

    Instance->InitParameters(InEmitterParent, InComponent);

    // CreateDistribution();
    
    return Instance;
}

void UParticleModuleLifetime::SetToSensibleDefaults(UParticleEmitter* Owner)
{
    // UDistributionFloatUniform* LifetimeDist = Cast<UDistributionFloatUniform>(Lifetime.Distribution);
    // if (LifetimeDist)
    // {
    //     LifetimeDist->Min = 1.0f;
    //     LifetimeDist->Max = 1.0f;
    //     LifetimeDist->bIsDirty = true;
    // }
}
