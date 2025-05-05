#include "SkinnedMeshComponent.h"

USkinnedMeshComponent::USkinnedMeshComponent()
    : Super(), SkeletalMesh(nullptr)
{

}

void USkinnedMeshComponent::SetSkeletalMesh(USkeletalMesh* InMesh)
{
    SkeletalMesh = InMesh;
    if (SkeletalMesh == nullptr)
    {
        OverrideMaterials.SetNum(0);
        AABB = FBoundingBox(FVector::ZeroVector, FVector::ZeroVector);
    }
    else 
    {
        OverrideMaterials.SetNum(SkeletalMesh->GetMaterials().Num());
        AABB = FBoundingBox(SkeletalMesh->GetRenderData()->BoundingBoxMin, SkeletalMesh->GetRenderData()->BoundingBoxMax);
    }
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

    FSkeletalMeshRenderData* rd = SkeletalMesh->GetRenderData();
    if (rd)
    {
        int32 NumRenderVerts = rd->Vertices.Num();
        //int32 NumSkinned = SkinnedPositions.Num();
        //int32 Count = FMath::Min(NumRenderVerts, NumSkinned);
        int32 Count = NumRenderVerts;
        for (int32 i = 0; i < Count; ++i)
        {
            // FSkeletalMeshVertex 와 FVector 의 대응
            rd->Vertices[i].X = SkinnedPositions[i].X;
            rd->Vertices[i].Y = SkinnedPositions[i].Y;
            rd->Vertices[i].Z = SkinnedPositions[i].Z;
        }
        // (필요하다면) 바운딩박스도 스킨드된 위치 기준으로 재계산 가능
    }

    FSkeletalMeshRenderData* afterRD = SkeletalMesh->GetRenderData();
}


const TArray<FVector>& USkinnedMeshComponent::GetSkinnedVertices() const
{
    return SkinnedPositions;
}




