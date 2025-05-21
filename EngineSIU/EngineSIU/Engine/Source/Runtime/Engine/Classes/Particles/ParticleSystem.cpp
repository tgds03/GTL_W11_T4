#include "ParticleSystem.h"

#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "TypeData/ParticleModuleTypeDataBase.h"

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
