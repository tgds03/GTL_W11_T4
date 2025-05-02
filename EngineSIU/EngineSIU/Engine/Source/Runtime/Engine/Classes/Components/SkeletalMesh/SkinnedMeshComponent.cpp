#include "SkinnedMeshComponent.h"

USkinnedMeshComponent::USkinnedMeshComponent()
    : SkeletalMesh(nullptr)
{

}

void USkinnedMeshComponent::SetSkeletalMesh(USkeletalMesh* InMesh)
{
    SkeletalMesh = InMesh;
}

void USkinnedMeshComponent::UpdateAnimation()
{
    if (!SkeletalMesh)
    {
        SkinnedPositions.Empty();
        return;
    }

    // 1. Update skeletal hierarchy global transforms
    SkeletalMesh->UpdateGlobalTransforms();

    // 2. Skin all vertices and cache positions
    SkinnedPositions = SkeletalMesh->SkinVertices();
}

void USkinnedMeshComponent::TickComponent(float DeltaTime)
{
    UpdateAnimation();

}

const TArray<FVector>& USkinnedMeshComponent::GetSkinnedVertices() const
{
    return SkinnedPositions;
}




