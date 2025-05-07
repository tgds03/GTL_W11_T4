#include "SkeletalMesh.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Asset/SkeletalMeshAsset.h"
#include "Engine/FObjLoader.h"

#include "UObject/Casts.h"
#include "Engine/Source/Runtime/CoreUObject/UObject/ObjectFactory.h"

//void USkeletalMesh::InitializeSkeleton(TArray<FBone>& BoneData)
//{
//    // Clear previous child lists
//    for (auto& Bone : Skeleton.Bones)
//    {
//        Bone.Children.Empty();
//    }
//
//    Skeleton.Bones = BoneData;
//    
//    // Rebuild parent-child links
//    for (int i = 0; i < (int)Skeleton.Bones.Num(); ++i)
//    {
//        int parent = Skeleton.Bones[i].ParentIndex;
//        if (parent >= 0 && parent < (int)Skeleton.Bones.Num())
//        {
//            Skeleton.Bones[parent].AddChild(i);
//        }
//    }
//    Skeleton.ComputeGlobalTransforms();
//    //Skeleton.SetInvBindTransforms();    // Inv는 기본포즈에서의 Global의 역행렬
//}

void USkeletalMesh::SetBoneLocalTransform(int boneIndex, const FBonePose& localTransform)
{
    if (boneIndex >= 0 && boneIndex < RenderData->Skeleton.BoneCount)
    {
        RenderData->Skeleton.Bones[boneIndex].LocalTransform = localTransform;
    }

    // GlobalTransform 갱신은 Render 찍기 전에만 딱 한번 하도록 하기!
}

void USkeletalMesh::UpdateGlobalTransforms()
{
    RenderData->Skeleton.ComputeGlobalTransforms();
}


UObject* USkeletalMesh::Duplicate(UObject* InOuter)
{
    ThisClass* NewSkeletalMesh = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewSkeletalMesh->RenderData = this->RenderData->Duplicate();
    NewSkeletalMesh->materials.Reserve(this->materials.Num());

    for (const auto& M : this->materials)
    {
        NewSkeletalMesh->materials.Emplace(M);
    }

    return nullptr;
}

USkeletalMesh* USkeletalMesh::DuplicateSkeletalMesh()
{
    ThisClass* NewSkeletalMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);

    NewSkeletalMesh->RenderData = this->RenderData->Duplicate();
    NewSkeletalMesh->materials.Reserve(this->materials.Num());

    for (const auto& M : this->materials)
    {
        NewSkeletalMesh->materials.Emplace(M);
    }

    return NewSkeletalMesh;
}

uint32 USkeletalMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Num(); materialIndex++) {
        if (materials[materialIndex]->MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void USkeletalMesh::GetUsedMaterials(TArray<UMaterial*>& OutMaterial) const
{
    for (const FStaticMaterial* Material : materials)
    {
        OutMaterial.Emplace(Material->Material);
    }
}

FSkeleton* USkeletalMesh::GetSkeleton() const
{
    return &(RenderData->Skeleton);
}

FWString USkeletalMesh::GetOjbectName() const
{
    return RenderData->ObjectName;
}

void USkeletalMesh::SetData(FSkeletalMeshRenderData* InRenderData)
{
    RenderData = InRenderData;

    for (int materialIndex = 0; materialIndex < RenderData->Materials.Num(); materialIndex++)
    {
        FStaticMaterial* newMaterialSlot = new FStaticMaterial();
        
        UMaterial* newMaterial = FResourceManager::CreateMaterial(RenderData->Materials[materialIndex]);

        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->MaterialSlotName = RenderData->Materials[materialIndex].MaterialName;

        materials.Add(newMaterialSlot);
    }
}


