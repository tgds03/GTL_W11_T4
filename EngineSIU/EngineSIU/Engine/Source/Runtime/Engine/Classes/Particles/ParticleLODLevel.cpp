#include "ParticleLODLevel.h"

#include "ParticleModule.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleSpawn.h"
#include "ParticleModuleSpawnBase.h"
#include "ParticleSpriteEmitter.h"
#include "Asset/StaticMeshAsset.h"
#include "Components/Mesh/StaticMeshRenderData.h"
#include "TypeData/ParticleModuleTypeDataBase.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

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
        }
        else if (Module->IsA<UParticleModuleSpawnBase>())
        {
            UParticleModuleSpawnBase* SpawnBase = Cast<UParticleModuleSpawnBase>(Module);
            SpawningModules.Add(SpawnBase);
        }
    }

    if (TypeDataModuleIndex != -1)
    {
        Modules.RemoveAt(TypeDataModuleIndex, 1);
    }

    if (TypeDataModule)
    {
        UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(TypeDataModule);
        if (MeshTD && MeshTD->Mesh && MeshTD->Mesh->GetRenderData())
        {
            UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(GetOuter());
            if (SpriteEmitter
                // && (MeshTD->bOverrideMaterial == false)
                )
            {
                FMaterialSubset& Section = MeshTD->Mesh->GetRenderData()->MaterialSubsets[0];
                if (MeshTD->Mesh->GetMaterials().Num() > Section.MaterialIndex)
                {
                    UMaterial* Material = MeshTD->Mesh->GetMaterials()[Section.MaterialIndex]->Material;
                    if (Material)
                    {
                        RequiredModule->Material = Material;
                    }
                }
            } 
        }
    }
}

bool UParticleLODLevel::InsertModule(UClass* InStaticClass, UParticleEmitter* TargetEmitter)
{
    for (int32 i = 0; i < Modules.Num(); i++)
    {
        if (Modules[i]->StaticClass() == InStaticClass)
        {
            return false;
        }
    }
    
    UParticleModule* Module = static_cast<UParticleModule*>(FObjectFactory::ConstructObject(InStaticClass, this));

    if ((SpawnModule == Module) ||
        (RequiredModule == Module))
    {
        return false;
    }

    if (Module->IsA(UParticleModuleTypeDataBase::StaticClass()))
    {
        TypeDataModule = CastChecked<UParticleModuleTypeDataBase>(Module);
    }
    else if (Module->IsA(UParticleModuleSpawn::StaticClass()))
    {
        SpawnModule = CastChecked<UParticleModuleSpawn>(Module);
    }
    else if (Module->IsA(UParticleModuleRequired::StaticClass()))
    {
        RequiredModule = CastChecked<UParticleModuleRequired>(Module);
    }
    else
    {
        Modules.Add(Module); 
    }

    TargetEmitter->UpdateModuleLists();

}
