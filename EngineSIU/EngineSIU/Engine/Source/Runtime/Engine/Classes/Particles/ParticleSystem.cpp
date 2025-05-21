#include "ParticleSystem.h"

#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "TypeData/ParticleModuleTypeDataBase.h"
#include "UObject/ObjectFactory.h"

UParticleSystem::UParticleSystem()
    : Delay(1)
    , DelayLow(0)
    , bUseDelayRange(false)
    , WarmupTime(0)
    , WarmupTickRate(30)
{
}

void UParticleSystem::UpdateAllModuleLists()
{
    for (int32 EmitterIdx = 0; EmitterIdx < Emitters.Num(); EmitterIdx++)
    {
        UParticleEmitter* Emitter = Emitters[EmitterIdx];
        if (Emitter != NULL)
        {
            for (int32 LODIdx = 0; LODIdx < Emitter->LODLevels.Num(); LODIdx++)
            {
                UParticleLODLevel* LODLevel = Emitter->LODLevels[LODIdx];
                if (LODLevel != NULL)
                {
                    LODLevel->UpdateModuleLists();
                }
            }

            // Allow type data module to cache any module info
            // only use for beam emitter
            // if(Emitter->LODLevels.Num() > 0)
            // {
            //     UParticleLODLevel* HighLODLevel = Emitter->LODLevels[0];
            //     if (HighLODLevel != nullptr && HighLODLevel->TypeDataModule != nullptr)
            //     {
            //         // Allow TypeData module to cache pointers to modules
            //         HighLODLevel->TypeDataModule->CacheModuleInfo(Emitter);
            //     }
            // }

            // Update any cached info from modules on the emitter
            Emitter->CacheEmitterModuleInfo();
        }
    }
}

void UParticleSystem::DeleteEmitterAt(int32 TargetEmitterIndex)
{
    DeleteEmitter(Emitters[TargetEmitterIndex]);
}

void UParticleSystem::DeleteEmitter(UParticleEmitter* TargetEmitter)
{
    for (auto& LODLevel : TargetEmitter->LODLevels)
    {
        for (auto& Module : LODLevel->Modules)
        {
            if (Module == LODLevel->TypeDataModule || Module == LODLevel->RequiredModule || Module == LODLevel->SpawnModule)
            {
                continue;
            }
            GUObjectArray.MarkRemoveObject(Module);
        }

        if (LODLevel->TypeDataModule)
        {
            GUObjectArray.MarkRemoveObject(LODLevel->TypeDataModule);
            LODLevel->TypeDataModule = nullptr;
        }
        GUObjectArray.MarkRemoveObject(LODLevel->RequiredModule);
        GUObjectArray.MarkRemoveObject(LODLevel->SpawnModule);
        
        LODLevel->RequiredModule = nullptr;
        LODLevel->SpawnModule = nullptr;
        GUObjectArray.MarkRemoveObject(LODLevel);
    }

    auto prevNum = Emitters.Num();
    Emitters.Remove(TargetEmitter);
    if (prevNum == Emitters.Num())
    {
        MessageBox(nullptr, L"왜 이전이랑 같음ㄴ", L"Error", MB_OK | MB_ICONERROR);
    }
    GUObjectArray.MarkRemoveObject(TargetEmitter);
}
