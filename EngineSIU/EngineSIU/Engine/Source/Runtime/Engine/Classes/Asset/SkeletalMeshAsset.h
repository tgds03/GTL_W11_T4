#pragma once

#include "Define.h"
#include "Hal/PlatformType.h"
#include "Container/Array.h"
#include "Launch/SkeletalDefine.h"

#define MAX_BONE_NUM 256

// 현재는 StaticMeshAsset과 동일한 내용
// 하지만 이후에 변경 가능하기에 분리해두었습니다.

struct FSkeletalMeshVertex
{
    float X, Y, Z;    // Position
    float R, G, B, A; // Color
    float NormalX, NormalY, NormalZ;
    float TangentX, TangentY, TangentZ, TangentW;
    float U = 0, V = 0;
    uint32 MaterialIndex;
    int BoneIndices[4];
    float BoneWeights[4];

    FVector GetSkinnedPosition(FSkeletonPose* SkeletonPose)
    {
        FVector WeightPosition = FVector::ZeroVector;

        for (int i = 0; i < 4; i++) {
            if (BoneIndices[i] < 0) continue;
            FMatrix SkinMat = SkeletonPose->GetSkinningMatrix(BoneIndices[i]);
            WeightPosition += SkinMat.TransformPosition(FVector(X, Y, Z));
        }

        return WeightPosition;
    }
};

struct FSkeletalMeshRenderData
{
    FWString ObjectName;
    FString DisplayName;

    TArray<FSkeletalMeshVertex> Vertices;
    TArray<UINT> Indices;

    TArray<FObjMaterialInfo> Materials;
    TArray<FMaterialSubset> MaterialSubsets;

    FVector BoundingBoxMin;
    FVector BoundingBoxMax;

    //FSkeletalMeshRenderData* Duplicate() const
    //{
    //    auto* Dst = new FSkeletalMeshRenderData();

    //    // 이름 복사
    //    Dst->ObjectName = ObjectName;
    //    Dst->DisplayName = DisplayName;

    //    // 배열 복사: Emplace를 이용해 각 요소를 깊은 복사
    //    Dst->Vertices.Reserve(Vertices.Num());
    //    for (const auto& V : Vertices)
    //    {
    //        Dst->Vertices.Emplace(V);
    //    }

    //    Dst->Indices.Reserve(Indices.Num());
    //    for (const auto& I : Indices)
    //    {
    //        Dst->Indices.Emplace(I);
    //    }

    //    Dst->Materials.Reserve(Materials.Num());
    //    for (const auto& M : Materials)
    //    {
    //        Dst->Materials.Emplace(M);
    //    }

    //    Dst->MaterialSubsets.Reserve(MaterialSubsets.Num());
    //    for (const auto& S : MaterialSubsets)
    //    {
    //        Dst->MaterialSubsets.Emplace(S);
    //    }

    //    // 스켈레톤 복사 (FSkeleton에 operator=가 구현되어 있다고 가정)
    //    Dst->Skeleton.BoneCount = Skeleton.BoneCount;
    //    Dst->Skeleton.Bones.Reserve(Skeleton.BoneCount);
    //    for (const auto& S : Skeleton.Bones)
    //    {
    //        Dst->Skeleton.Bones.Emplace(S);
    //    }

    //    // 바운딩 박스 복사
    //    Dst->BoundingBoxMin = BoundingBoxMin;
    //    Dst->BoundingBoxMax = BoundingBoxMax;

    //    return Dst;
    //}
};

struct FBoneWeightConstants
{
    FMatrix BoneTransform[MAX_BONE_NUM];
};
