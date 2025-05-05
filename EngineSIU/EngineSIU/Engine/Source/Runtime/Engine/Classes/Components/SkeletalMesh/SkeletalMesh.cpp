#include "SkeletalMesh.h"

#include "Engine/FObjLoader.h"



void USkeletalMesh::InitializeSkeleton(TArray<FBone>& BoneData)
{
    Skeleton.Bones = BoneData;
    // Clear previous child lists
    for (auto& Bone : Skeleton.Bones)
    {
        Bone.Children.Empty();
    }
    // Rebuild parent-child links
    for (int i = 0; i < (int)Skeleton.Bones.Num(); ++i)
    {
        int parent = Skeleton.Bones[i].ParentIndex;
        if (parent >= 0 && parent < (int)Skeleton.Bones.Num())
        {
            Skeleton.Bones[parent].AddChild(i);
        }
    }
}

void USkeletalMesh::SetBoneLocalTransform(int boneIndex, const FMatrix& localTransform)
{
    if (boneIndex >= 0 && boneIndex < (int)Skeleton.Bones.Num())
    {
        Skeleton.Bones[boneIndex].LocalTransform = localTransform;
    }

    Skeleton.ComputeGlobalTransforms();
    Skeleton.SetInvBindTransforms();
}

void USkeletalMesh::UpdateGlobalTransforms()
{
    Skeleton.ComputeGlobalTransforms();
}

TArray<FVector> USkeletalMesh::SkinVertices() const
{
    TArray<FVector> SkinnedPositions;
    for (const auto& Vertex : SourceVertices)
    {
        SkinnedPositions.Add(Vertex.SkinVertexPosition(Skeleton));
    }
    return SkinnedPositions;
}

UObject* USkeletalMesh::Duplicate(UObject* InOuter)
{
    // TODO: Context->CopyResource를 사용해서 Buffer복사
    // ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate());
    return nullptr;
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
        
        UMaterial* newMaterial = FObjManager::CreateMaterial(RenderData->Materials[materialIndex]);

        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->MaterialSlotName = RenderData->Materials[materialIndex].MaterialName;

        materials.Add(newMaterialSlot);
    }
}


