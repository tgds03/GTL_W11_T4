#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Launch/SkeletalDefine.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"

class USkeletalMesh : public UObject
{
    DECLARE_CLASS(USkeletalMesh, UObject)

public:
    USkeletalMesh() = default;
    /** Internal skeleton data */
    FSkeleton Skeleton;

    /** Mesh vertex array (pre-skinning) */
    TArray<FVertexSkeletal> SourceVertices;

    void InitializeSkeleton(TArray<FBone>& BoneData);

    /** Set the local transform for a specific bone by index */
    void SetBoneLocalTransform(int boneIndex, const FMatrix& localTransform);

    /** Recompute global transforms for all bones */
    void UpdateGlobalTransforms();

    /**
     * Skin all source vertices and return their skinned positions.
     */
    TArray<FVector> SkinVertices() const;
};
