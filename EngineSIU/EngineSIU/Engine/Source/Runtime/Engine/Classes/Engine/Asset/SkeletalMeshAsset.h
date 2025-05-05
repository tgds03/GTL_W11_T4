#pragma once

#include "Define.h"
#include "Hal/PlatformType.h"
#include "Container/Array.h"

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
};

struct FSkeletalMeshRenderData
{
    FWString ObjectName;
    FString DisplayName;

    TArray<FSkeletalMeshVertex> Vertices;
    TArray<UINT> Indices;

    TArray<FObjMaterialInfo> Materials;
    TArray<FMaterialSubset> MaterialSubsets;

    //FSkeleton Skeleton;

    FVector BoundingBoxMin;
    FVector BoundingBoxMax;
};
