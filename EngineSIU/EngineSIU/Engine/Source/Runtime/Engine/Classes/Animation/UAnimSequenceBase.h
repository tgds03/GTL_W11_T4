#pragma once
#include "Animation/UAnimationAsset.h"
#include "Animation/AnimTypes.h"
#include "UAnimDataModel.h"

class UAnimSequenceBase : public UAnimationAsset
{
    TArray<FAnimNotifyEvent> Notifies;

    float RateScale;

    UAnimDataModel* AnimDataModel;

public:
    virtual ~UAnimSequenceBase() = default;


    UAnimDataModel* GetDataMode() const { return AnimDataModel; }

    // 노티파이 접근자
    const TArray<FAnimNotifyEvent>& GetNotifies() const { return Notifies; }
    void AddNotify(const FAnimNotifyEvent& Notify) { Notifies.Add(Notify); }
    UAnimSequenceBase();
    bool RemoveNotify(int32 NotifyIndex);

    // 노티파이 검색
    bool FindNotifyEvent(float Time, FAnimNotifyEvent& OutEvent) const;

    // 재생 속도 접근자
    float GetRateScale() const { return RateScale; }
    void SetRateScale(float InRate) { RateScale = FMath::Max(0.01f, InRate); }

    // 재생 길이 조정 (재생 속도 적용)
    virtual float GetPlayLength() const override { return 0.0f; }
};

