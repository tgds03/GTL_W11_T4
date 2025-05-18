#include "ParticleModule.h"
#include "ParticleHelper.h"
#include "ParticleEmitterInstances.h"
#include "ParticleSystemComponent.h"

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
        if (InRandSeedInfo.bGetSeedFromInstance == true)
        {
            float SeedValue;
            if (Owner->Component->GetFloatParameter(InRandSeedInfo.ParameterName, SeedValue) == true)
            {
                if (InRandSeedInfo.bInstanceSeedIsIndex == false)
                {
                    InRandSeedPayload->RandomStream.Initialize(static_cast<int32>(std::round(SeedValue)));
                }
                else
                {
                    if (InRandSeedInfo.RandomSeeds.Num() > 0)
                    {
                        int32 Index = FMath::Min<int32>((InRandSeedInfo.RandomSeeds.Num() - 1), static_cast<int32>(std::trunc(SeedValue)));
                        InRandSeedPayload->RandomStream.Initialize(InRandSeedInfo.RandomSeeds[Index]);
                        return 0;
                    }
                    else
                    {
                        return 0xffffffff;
                    }
                }
                return 0;
            }
        }

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

