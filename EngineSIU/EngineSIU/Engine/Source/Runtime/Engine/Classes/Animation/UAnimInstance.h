#pragma once

#include "UAnimDataModel.h"
#include "AnimSequence.h"
#include <Components/SkeletalMesh/SkeletalMeshComponent.h>

class UAnimInstance : public UObject
{
protected:
    // 소유 컴포넌트
    USkeletalMeshComponent* OwningComponent;

    UAnimSequence* CurrentSequence;

    // 재생 상태
    bool bIsPlaying;
    bool bLooping;
    float CurrentTime;
    float PlayRate;

    // 현재 포즈 데이터
    //TArray<FTransform> CurrentPose;

public:
    UAnimInstance();
    virtual ~UAnimInstance() = default;

    // 컴포넌트와 연결
    void Initialize(USkeletalMeshComponent* InComponent);

    // 매 프레임 업데이트
    void Update(float DeltaTime);

    // 애니메이션 재생 제어
    void PlayAnimation(UAnimSequence* InSequence, bool bInLooping = false);
    void StopAnimation();
    void PauseAnimation();
    void ResumeAnimation();

    // 현재 애니메이션 접근자
    UAnimSequence* GetCurrentAnimation() const { return CurrentSequence; }

    // 현재 포즈 접근자
   // const TArray<FTransform>& GetCurrentPose() const { return CurrentPose; }

    // 재생 상태 접근자
    bool IsPlaying() const { return bIsPlaying; }
    float GetCurrentTime() const { return CurrentTime; }
    void SetCurrentTime(float InTime);

    // 재생 속도 설정
    void SetPlayRate(float InRate) { PlayRate = FMath::Max(0.01f, InRate); }
    float GetPlayRate() const { return PlayRate; }

protected:
    // 애니메이션 노티파이 처리
    void ProcessNotifies(float PreviousTime, float CurrentTime);

    // 애니메이션 상태 업데이트
    void UpdateAnimationState(float DeltaTime);

    // 포즈 계산
    void CalculatePose();
};
