
#include "CameraShakeBase.h"

UCameraShakeBase::UCameraShakeBase()
    : ShakeScale(1.0f)
    , RootShakePattern(nullptr)
    , CameraManager(nullptr)
    , bIsActive(true)
{
}

void UCameraShakeBase::UpdateAndApplyCameraShake(float DeltaTime, float Alpha, FMinimalViewInfo& InOutPOV)
{
    if (RootShakePattern)
    {
        FCameraShakePatternUpdateParams Params(InOutPOV);
        Params.DeltaTime = DeltaTime;
        Params.ShakeScale = ShakeScale;
        Params.DynamicScale = Alpha;

        FCameraShakePatternUpdateResult Result;

        // 쉐이크 업데이트
        RootShakePattern->UpdateShakePattern(Params, Result);

        if (!RootShakePattern->IsFinished())
        {
            // 쉐이크 적용
            ApplyResult(Params.GetTotalScale(), Result, InOutPOV);
        }
    }
}

void UCameraShakeBase::StartShake()
{
}

void UCameraShakeBase::StopShake(bool bImmediately)
{
    if (RootShakePattern)
    {
        RootShakePattern->StopShakePattern(bImmediately);
    }
}

void UCameraShakeBase::ApplyResult(float Scale, const FCameraShakePatternUpdateResult& InResult, FMinimalViewInfo& InOutPOV)
{
    // InResult를 기반으로 InOutPOV 업데이트
    
}

void UCameraShakePattern::UpdateShakePattern(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult)
{
    UpdateShakePatternImpl(Params, OutResult);
}

bool UCameraShakePattern::IsFinished() const
{
    return IsFinishedImpl();
}

void UCameraShakePattern::StopShakePattern(bool bImmediately)
{
    StopShakePatternImpl(bImmediately);
}
