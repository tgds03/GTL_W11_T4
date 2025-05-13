#pragma once
#include "Animation/UAnimationAsset.h"
#include "Animation/AnimTypes.h"
#include "UAnimDataModel.h"

class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset);

    TArray<FAnimNotifyEvent> Notifies;
    UAnimDataModel* AnimDataModel = nullptr;
    float RateScale = 1.0f;
    float AnimationStartTime = 0.0f;
    
    
    // 루프 애니메이션 여부
    bool bLoopAnimation;

    // 루트 모션 사용 여부
    bool bEnableRootMotion;

public:
    UAnimSequenceBase();
    virtual ~UAnimSequenceBase() = default;
    
    UAnimDataModel* GetDataModel() const { return AnimDataModel; }
    void SetDataModel(UAnimDataModel* AnimDataModel) { this->AnimDataModel = AnimDataModel; }

    bool IsLooping() const { return bLoopAnimation; }
    void SetLooping(bool bLoop) { bLoopAnimation = bLoop; }
    bool HasRootMotion() const { return bEnableRootMotion; }
    void SetEnableRootMotion(bool bEnable) { bEnableRootMotion = bEnable; }
    int32 GetNumberOfFrames() const;

    // 노티파이 접근자
    const TArray<FAnimNotifyEvent>& GetNotifies() const { return Notifies; }
    void AddNotify(const FAnimNotifyEvent& Notify) { Notifies.Add(Notify); }
    bool RemoveNotify(int32 NotifyIndex);

    // 노티파이 검색
    bool FindNotifyEvent(float Time, FAnimNotifyEvent& OutEvent) const;

    // 재생 속도 접근자
    float GetRateScale() const { return RateScale; }
    void SetRateScale(float InRate) { RateScale = InRate; }

    // 재생 길이 조정 (재생 속도 적용)
    float GetUnScaledPlayLength() const;
};

