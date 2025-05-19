#include "ParticleLODLevel.h"

#include "ParticleModule.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleSpawnBase.h"
#include "TypeData/ParticleModuleTypeDataBase.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"
#include "UObject/Casts.h"

UParticleModule* UParticleLODLevel::GetModuleAtIndex(int32 InIndex)
{
    // 'Normal' modules
    if (InIndex > INDEX_NONE)
    {
        if (InIndex < Modules.Num())
        {
            return Modules[InIndex];
        }

        return NULL;
    }

    switch (InIndex)
    {
    case INDEX_REQUIREDMODULE:		return RequiredModule;
    case INDEX_SPAWNMODULE:			return SpawnModule;
    case INDEX_TYPEDATAMODULE:		return TypeDataModule;
    }

    return NULL;
}

void UParticleLODLevel::UpdateModuleLists()
{
    SpawningModules.Empty();
    SpawnModules.Empty();
    UpdateModules.Empty();
    // OrbitModules.Empty();
    // EventReceiverModules.Empty();
    // EventGenerator = NULL;

    UParticleModule* Module;
    int32 TypeDataModuleIndex = -1;

    for (int32 i = 0; i < Modules.Num(); ++i)
    {
        Module = Modules[i];
        if (!Module)
            continue;

        if (Module->GetFlag(EModuleFlag::SpawnModule))
        {
            SpawnModules.Add(Module);
        }
        if (Module->GetFlag(EModuleFlag::UpdateModule) || Module->GetFlag(EModuleFlag::FinalUpdateModule))
        {
            UpdateModules.Add(Module);
        }

        if (Module->IsA<UParticleModuleTypeDataBase>())
        {
            TypeDataModule = Cast<UParticleModuleTypeDataBase>(Module);
            if (TypeDataModule && !TypeDataModule->GetFlag(EModuleFlag::SpawnModule) && TypeDataModule->GetFlag(EModuleFlag::UpdateModule))
            {
                TypeDataModuleIndex = i;
            }
        } else if (Module->IsA<UParticleModuleSpawnBase>())
        {
            UParticleModuleSpawnBase* SpawnModule = Cast<UParticleModuleSpawnBase>(Module);
            SpawnModules.Add(SpawnModule);
        }
    }

    if (TypeDataModuleIndex != -1)
    {
        Modules.RemoveAt(TypeDataModuleIndex, 1);
    }

    if (TypeDataModule)
    {
        UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(TypeDataModule);
        if (MeshTD && MeshTD->Mesh /** && MeshTD->Mesh->HasValidRenderData(false) */)
        {
            // UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(GetOuter());
            // if (SpriteEmitter && (MeshTD->bOverrideMaterial == false))
            // {
            //     FStaticMeshSection& Section = MeshTD->Mesh->GetRenderData()->LODResources[0].Sections[0];
            //     UMaterialInterface* Material = MeshTD->Mesh->GetMaterial(Section.MaterialIndex);
            //     if (Material)
            //     {
            //         RequiredModule->Material = Material;
            //     }
            // } 
        }
    }
}
