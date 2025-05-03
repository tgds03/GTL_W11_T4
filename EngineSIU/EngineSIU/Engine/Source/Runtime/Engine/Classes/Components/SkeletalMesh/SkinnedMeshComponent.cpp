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
    // TODO UpdateAnimation의 위치 변경
    // 일단 매 프레임마다 UpdateAnimation을 걸어주긴 하지만
    // 추후 관절이 움직일 때만 UpdateAnimation을 해주어야함
    UpdateAnimation();

}

const TArray<FVector>& USkinnedMeshComponent::GetSkinnedVertices() const
{
    return SkinnedPositions;
}




