#include "UAnimSequenceBase.h"

#include <algorithm>


UAnimSequenceBase::UAnimSequenceBase()
    : RateScale(1.0f)
{
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

void UAnimSequenceBase::AddNotify(float Time, FName Name)
{
    FAnimNotifyEvent NewNotify;
    NewNotify.TriggerTime = Time;
    NewNotify.NotifyName = Name;
    Notifies.Add(NewNotify);
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
