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
        float TotalWeight = 0.0f;

        for (int i = 0; i < 4; i++) {
            // 유효한 본과 의미 있는 가중치만 처리
            if (BoneIndices[i] < 0 || BoneWeights[i] < 0.001f)
                continue;

            FMatrix SkinMat = SkeletonPose->GetSkinningMatrix(BoneIndices[i]);

            // 가중치 적용!
            WeightPosition += SkinMat.TransformPosition(FVector(X, Y, Z)) * BoneWeights[i];
            TotalWeight += BoneWeights[i];
        }

        // 가중치 합이 0이 아니고 1과 크게 차이나면 정규화
        if (TotalWeight > 0.0f && FMath::Abs(TotalWeight - 1.0f) > 0.01f) {
            WeightPosition /= TotalWeight;
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

    TArray<FSkeletalMeshVertex> CPUSkinnedVertices;
};

struct FBoneWeightConstants
{
    FMatrix BoneTransform[MAX_BONE_NUM];
};
