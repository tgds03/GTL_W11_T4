#pragma once

#include "Runtime/CoreUObject/UObject/Object.h"
#include "UObject/ObjectMacros.h"
class FSkeleton;

class UAnimationAsset : public UObject
{
    DECLARE_CLASS(UAnimationAsset, UObject);

protected:
    // 이 애니메이션이 사용하는 스켈레톤
    FSkeleton* Skeleton;

public:
    UAnimationAsset();
    virtual ~UAnimationAsset() = default;

    // 스켈레톤 접근자
    FSkeleton* GetSkeleton() const { return Skeleton; }
    void SetSkeleton(FSkeleton* InSkeleton) { Skeleton = InSkeleton; }

    // 재생 길이 - 가상 함수, 자식 클래스에서 구현
    virtual float GetPlayLength() const { return 0.0f; }
};

