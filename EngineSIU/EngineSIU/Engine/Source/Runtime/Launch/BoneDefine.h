#pragma once

#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Math/Quat.h"
#include "Math/JungleMath.h"

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
        Location = M.GetTranslationVector();

        Scale = M.GetScaleVector();

        FMatrix RotOnly = M.GetMatrixWithoutScale();
        Rotation = RotOnly.ToQuat();
    }

    FMatrix ToMatrix() 
    {
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

    /** 본의 바인드 트랜스폼의 역행렬 (스켈레톤의 원본 위치) */
    FMatrix InvBindTransform = FMatrix::Identity;

    FBone() = default;
    FBone(const FName& InName, const int32 InParentIndex)
        : Name(InName), ParentIndex(InParentIndex)
    {
    }

    /** 자식 본 추가 */
    void AddChildIndex(int32 ChildIndex)
    {
        Children.Add(ChildIndex);
    }
};
