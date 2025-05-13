#pragma once

#include "Animation/UAnimDataModel.h"
#include "Animation/UAnimSequenceBase.h"
#include "SkeletalDefine.h"

// UAnimSequence 클래스 정의
class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UAnimSequenceBase)
public:
    UAnimSequence();
    virtual ~UAnimSequence() = default;

    float GetLocalTime(float GlobalTime) const;

    void StartAnimSequence(float InStartTime);
    
    // 특정 시간에 포즈 계산
    virtual void GetAnimationPose(float Time, USkeletalMesh* SkeletalMesh, TArray<FBonePose>& OutBoneTransforms) const;

private:
    // 키프레임 보간 헬퍼 함수
    void InterpolateKeyframe(
        const FRawAnimSequenceTrack& Track,
        int32 Frame1,
        int32 Frame2,
        float Alpha,
        FVector& OutPosition,
        FQuat& OutRotation,
        FVector& OutScale
    ) const;
};
