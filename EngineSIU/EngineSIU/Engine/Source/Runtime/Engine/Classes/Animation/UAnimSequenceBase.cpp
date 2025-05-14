#include "UAnimSequenceBase.h"

#include <algorithm>

#include "UObject\Casts.h"


UAnimSequenceBase::UAnimSequenceBase()
    : RateScale(1.0f)
{
}

UObject* UAnimSequenceBase::Duplicate(UObject* InOuter)
{
    UAnimSequenceBase* NewAnimSequence = Cast<UAnimSequenceBase>(Super::Duplicate(InOuter));

    NewAnimSequence->Notifies.Empty();
    NewAnimSequence->Notifies.Reserve(Notifies.Num());
    for (const FAnimNotifyEvent& OrigEvent : Notifies)
    {
        FAnimNotifyEvent NewEvent = OrigEvent;
        NewAnimSequence->Notifies.Add(NewEvent);
    }

    NewAnimSequence->Notifies = Notifies;
    NewAnimSequence->RateScale = RateScale;
    NewAnimSequence->LocalTime = LocalTime;
    NewAnimSequence->bLoopAnimation = bLoopAnimation;
    NewAnimSequence->bEnableRootMotion = bEnableRootMotion;
    NewAnimSequence->SetSkeleton(Skeleton);
    NewAnimSequence->SetDataModel(AnimDataModel);

    return NewAnimSequence;
}

int32 UAnimSequenceBase::GetNumberOfFrames() const
{
    if (bLoopAnimation)
    {
        return AnimDataModel->GetNumberOfFrames();
    }
    else
    {
        return FMath::Max(AnimDataModel->GetNumberOfFrames() - 1, 0);;
    }
}

bool UAnimSequenceBase::RemoveNotify(int32 NotifyIndex)
{
    if (Notifies.IsValidIndex(NotifyIndex))
    {
        Notifies.RemoveAt(NotifyIndex);
        return true;
    }
    return false;
}

void UAnimSequenceBase::BeginSequence()
{
    LocalTime = 0.f;
}

void UAnimSequenceBase::TickSequence(float DeltaTime)
{
    float SequenceLength = GetUnScaledPlayLength();

    if (FMath::IsNearlyZero(SequenceLength))
    {
        LocalTime = 0.f;
        return;
    }

    float RawLocalTime = LocalTime + DeltaTime * RateScale;
    
    if (bLoopAnimation)
    {
        float TimeInCycle = FMath::Fmod(RawLocalTime, SequenceLength);

        if (TimeInCycle < 0.0f)
        {
            TimeInCycle += SequenceLength;
        }
        LocalTime = TimeInCycle;
    }
    // 루프가 아닌 경우
    else
    {
        LocalTime = FMath::Clamp(RawLocalTime, 0.0f, SequenceLength);
    }
}

bool UAnimSequenceBase::FindNotifyEvent(float Time, FAnimNotifyEvent& OutEvent) const
{
    for (const FAnimNotifyEvent& Notify : Notifies)
    {
        float EndTime = Notify.TriggerTime + Notify.Duration;

        if (Time >= Notify.TriggerTime && Time <= EndTime)
        {
            OutEvent = Notify;
            return true;
        }
    }
    return false;
}

float UAnimSequenceBase::GetUnScaledPlayLength() const
{
    if (!AnimDataModel)
    {
        return 0.0f;
    }

    return FMath::Abs(AnimDataModel->GetPlayLength());
}
