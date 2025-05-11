#include "UAnimSequenceBase.h"


UAnimSequenceBase::UAnimSequenceBase()
    : RateScale(1.0f)
{
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
