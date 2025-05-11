#pragma once

#include <fbxsdk.h>
#include "Container/Map.h"
#include "UObject/Object.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"

struct FSkeletalMeshRenderData;
struct FSkeletalMeshVertex;

struct FFbxLoader 
{
public:
    static USkeletalMesh* LoadFBXSkeletalMeshAsset(const FString& filePathName);

    static bool CreateTextureFromFile(const FWString& Filename, bool bIsSRGB = true);

    static void CalculateTangent(FSkeletalMeshVertex& PivotVertex, const FSkeletalMeshVertex& Vertex1, const FSkeletalMeshVertex& Vertex2);

    static FbxVector4 GetNormalMappingVector(FbxGeometryElementNormal* normalElem, FbxMesh* mesh, int polygonIndex, int vertIndex);

    static void ComputeBoundingBox(const TArray<FSkeletalMeshVertex>& InVertices, FVector& OutMinVector, FVector& OutMaxVector);
};
