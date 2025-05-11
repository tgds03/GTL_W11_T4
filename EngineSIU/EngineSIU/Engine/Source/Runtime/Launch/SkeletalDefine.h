#pragma once

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"

#include "Engine/Source/Runtime/CoreUObject/UObject/NameTypes.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"
#include "Engine/Source/Runtime/Core/Math/Quat.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

#include "BoneDefine.h"

#define INDEX_NONE -1

// FSkeleton: 본 트리를 관리
class FSkeleton
{
public:
    /** 본 배열 */
    TArray<FBone> Bones;
    int32 BoneCount = 0;

public:
    FSkeleton() = default;
    FSkeleton(TArray<FBone> InBones) : Bones(InBones)
    {
        BoneCount = (int32)Bones.Num();

        for (auto& Bone : Bones)
        {
            Bone.Children.Empty();
        }

        for (int i = 0; i < BoneCount; ++i)
        {
            int parent = Bones[i].ParentIndex;
            if (parent >= 0 && parent < BoneCount)
            {
                Bones[parent].AddChildIndex(i);
            }
        }
    }

    // TODO: SkeltonPose의 LocalTransform과 GlobalTransform과 인덱스 맞춰줘야 함.
    // 아직은 쓰는 곳이 없어서 문제 없음.
    /** 본을 추가하고, 부모-자식 관계를 자동으로 연결 */
    int32 AddBone(const FName& BoneName, const int32 ParentIndex)
    {
        int32 NewIndex = Bones.Add(FBone(BoneName, ParentIndex));
        if (ParentIndex != INDEX_NONE && Bones.IsValidIndex(ParentIndex))
        {
            Bones[ParentIndex].AddChildIndex(NewIndex);
        }

        BoneCount = (int32)Bones.Num();

        return NewIndex;
    }
};

class FSkeletonPose
{
public:
    FSkeleton* Skeleton = nullptr;
    TArray<FBonePose> LocalTransforms;
    TArray<FMatrix> GlobalTransforms;

    FSkeletonPose() = default;

    FSkeletonPose(FSkeleton* InSkeleton)
        : Skeleton(InSkeleton)
    {
        LocalTransforms.SetNum(Skeleton->Bones.Num());
        GlobalTransforms.SetNum(Skeleton->Bones.Num());
    }

    FSkeletonPose(FSkeleton* InSkeleton, TArray<FBonePose> InLocalTransform)
        : FSkeletonPose(InSkeleton)
    {
        LocalTransforms = InLocalTransform;
        ComputeGlobalTransforms();
        SetInvBindTransforms();
    }

    FSkeletonPose Duplicate() const
    {
        FSkeletonPose CopyPose;

        // 1. Skeleton 복사
        FSkeleton* DuplicatedSKeleton = DuplicateSkeleton(Skeleton);

        CopyPose.Skeleton = DuplicatedSKeleton;
        CopyPose.LocalTransforms = LocalTransforms;
        CopyPose.GlobalTransforms = GlobalTransforms;
        
        return CopyPose;
    }

    TArray<FMatrix> GetSkinningMatrixPalette()
    {
        TArray<FMatrix> SkinningMatrices;
        for (int32 i = 0; i < Skeleton->Bones.Num(); ++i)
        {
            FMatrix SkinningMatrix = Skeleton->Bones[i].InvBindTransform * GlobalTransforms[i];
            SkinningMatrices.Add(SkinningMatrix);
        }

        return SkinningMatrices;
    }

    FMatrix GetSkinningMatrix(int32 BoneIndex)
    {
        if (Skeleton->Bones.IsValidIndex(BoneIndex))
        {
            return Skeleton->Bones[BoneIndex].InvBindTransform * GlobalTransforms[BoneIndex];
        }

        return FMatrix::Identity;
    }

    /** 루트부터 재귀적으로 GlobalTransform 계산 */
    void ComputeGlobalTransforms()
    {
        // 루트 본들(ParentIndex == INDEX_NONE)을 찾아서 재귀 호출
        for (int32 i = 0; i < Skeleton->Bones.Num(); ++i)
        {
            if (Skeleton->Bones[i].ParentIndex == INDEX_NONE)
            {
                ComputeGlobalTransformRecursive(i, FMatrix::Identity);
            }
        }
    }

private:
    FSkeleton* DuplicateSkeleton(const FSkeleton* OriginalData) const
    {
        FSkeleton* NewSkeleton = new FSkeleton();
        NewSkeleton->BoneCount = OriginalData->BoneCount;
        NewSkeleton->Bones = OriginalData->Bones;
        return NewSkeleton;  
    }

    /** 본 인덱스와 부모의 글로벌 트랜스폼을 받아 재귀 계산 */
    void ComputeGlobalTransformRecursive(int32 BoneIndex, const FMatrix& ParentGlobal)
    {
        FBone& Bone = Skeleton->Bones[BoneIndex];
        GlobalTransforms[BoneIndex] = LocalTransforms[BoneIndex].ToMatrix() * ParentGlobal;

        for (int32 ChildIdx : Bone.Children)
        {
            ComputeGlobalTransformRecursive(ChildIdx, GlobalTransforms[BoneIndex]);
        }
    }

    void SetInvBindTransforms()
    {
        for (int32 i = 0; i < Skeleton->Bones.Num(); ++i)
        {
            Skeleton->Bones[i].InvBindTransform = FMatrix::Inverse(GlobalTransforms[i]);
        }
    }
};
