#pragma once

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"

#include "Engine/Source/Runtime/CoreUObject/UObject/NameTypes.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"
#include "Engine/Source/Runtime/Core/Math/Quat.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

#define INDEX_NONE -1

struct FBonePose 
{
    FQuat Rotation;
    FVector Location;
    FVector Scale;

public:
    FBonePose() 
        :Rotation(FQuat()), Location(FVector::ZeroVector), Scale(FVector(1.0f, 1.0f, 1.0f))
    {
    }

    FBonePose(const FQuat& R, const FVector& L, const FVector& S)
        : Rotation(R), Location(L), Scale(S)
    {
    }

    FBonePose(const FMatrix& M)
    {
        // 1) 위치 추출
        Location = M.GetTranslationVector();

        // 2) 스케일 추출
        Scale = M.GetScaleVector();

        // 3) 스케일을 제거한 순수 회전 행렬 얻기
        FMatrix RotOnly = M.GetMatrixWithoutScale();

        // 4) 회전 행렬을 쿼터니언으로 변환
        Rotation = RotOnly.ToQuat();
    }

    FMatrix ToMatrix() {
        return JungleMath::CreateModelMatrix(Location, Rotation, Scale);
    }
};


// FBone: 트리(스켈레톤)상의 노드로 동작
struct FBone
{
    /** 이 본의 이름 */
    FName Name;

    /** 부모 본의 인덱스 (루트면 INDEX_NONE) */
    int32 ParentIndex = INDEX_NONE;

    /** 자식 본들의 인덱스 */
    TArray<int32> Children;

    /** 로컬 트랜스폼 (SRT 또는 4x4 매트릭스) */
    FBonePose LocalTransform;

    /** 글로벌 트랜스폼 캐시 (ComputeGlobalTransforms() 호출 시 계산) */
    FMatrix GlobalTransform = FMatrix::Identity;

    /** 본의 바인드 트랜스폼의 역행렬 (스켈레톤의 원본 위치) */
    FMatrix InvBindTransform = FMatrix::Identity;

    FBone() = default;
    FBone(const FName& InName, int32 InParentIndex, const FBonePose& InLocalTransform)
        : Name(InName), ParentIndex(InParentIndex), LocalTransform(InLocalTransform)
    {
    }

    /** 자식 본 추가 */
    void AddChild(int32 ChildIndex)
    {
        Children.Add(ChildIndex);
    }
};

// FSkeleton: 본 트리를 관리
class FSkeleton
{
public:
    /** 본 배열 */
    TArray<FBone> Bones;
    int32 BoneCount;

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
                Bones[parent].AddChild(i);
            }
        }

        ComputeGlobalTransforms();
        SetInvBindTransforms();    // Inv는 기본포즈에서의 Global의 역행렬
    }

    /** 본을 추가하고, 부모-자식 관계를 자동으로 연결 */
    int32 AddBone(const FName& BoneName, int32 ParentIndex, const FBonePose& LocalTransform)
    {
        int32 NewIndex = Bones.Add(FBone(BoneName, ParentIndex, LocalTransform));
        if (ParentIndex != INDEX_NONE && Bones.IsValidIndex(ParentIndex))
        {
            Bones[ParentIndex].AddChild(NewIndex);
        }
        return NewIndex;
    }

    /** 루트부터 재귀적으로 GlobalTransform 계산 */
    void ComputeGlobalTransforms()
    {
        // 루트 본들(ParentIndex == INDEX_NONE)을 찾아서 재귀 호출
        for (int32 i = 0; i < Bones.Num(); ++i)
        {
            if (Bones[i].ParentIndex == INDEX_NONE)
            {
                ComputeGlobalTransformRecursive(i, FMatrix::Identity);
            }
        }
    }

private:
    /** 본 인덱스와 부모의 글로벌 트랜스폼을 받아 재귀 계산 */
    void ComputeGlobalTransformRecursive(int32 BoneIndex, const FMatrix& ParentGlobal)
    {
        FBone& Bone = Bones[BoneIndex];
        Bone.GlobalTransform = Bone.LocalTransform.ToMatrix() * ParentGlobal;

        for (int32 ChildIdx : Bone.Children)
        {
            ComputeGlobalTransformRecursive(ChildIdx, Bone.GlobalTransform);
        }
    }

    void SetInvBindTransforms()
    {
        for (FBone& Bone : Bones)
        {
            // 본의 글로벌 트랜스폼을 역행렬로 설정
            Bone.InvBindTransform = FMatrix::Inverse(Bone.GlobalTransform);
        }
    }
};

