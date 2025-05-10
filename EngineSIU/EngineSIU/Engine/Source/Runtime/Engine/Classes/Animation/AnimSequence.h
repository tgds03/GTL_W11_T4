#pragma once

#include "Animation/UAnimDataModel.h"
#include "Animation/UAnimSequenceBase.h"

// UAnimSequence 클래스 정의
class UAnimSequence : public UAnimSequenceBase
{
private:
    // 루프 애니메이션 여부
    bool bLoopAnimation;

    // 루트 모션 사용 여부
    bool bEnableRootMotion;

public:
    UAnimSequence();
    virtual ~UAnimSequence() = default;

    // 루프 설정 접근자
    bool IsLooping() const { return bLoopAnimation; }
    void SetLooping(bool bLoop) { bLoopAnimation = bLoop; }

    // 루트 모션 접근자
    bool HasRootMotion() const { return bEnableRootMotion; }
    void EnableRootMotion(bool bEnable) { bEnableRootMotion = bEnable; }

    // 재생 길이 (재생 속도 적용, 부모 클래스 오버라이드)
    virtual float GetPlayLength() const override;

    // 특정 시간에 포즈 계산
    //void GetAnimationPose(float Time, TArray<FTransform>& OutBoneTransforms) const;
    // 메타데이터 접근
    int32 GetNumberOfFrames() const;
    FFrameRate GetFrameRate() const;

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
