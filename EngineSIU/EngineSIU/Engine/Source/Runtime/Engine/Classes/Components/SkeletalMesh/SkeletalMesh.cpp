#include "SkeletalMesh.h"

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


