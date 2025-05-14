#include "UAnimSequenceBase.h"


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
