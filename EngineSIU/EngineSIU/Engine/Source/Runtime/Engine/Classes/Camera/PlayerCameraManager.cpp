#include "PlayerCameraManager.h"

APlayerCameraManager::APlayerCameraManager()
{
    VignetteColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
    VignetteCenter = FVector2D(0.5f, 0.5f);
    VignetteRadius = 0.5f;
    VignetteSmoothness = 0.5f;
    VignetteIntensity = 0.0f;
    LetterBoxWidth = 2.35f;
    LetterBoxHeight = 1.0f;
}

AActor* APlayerCameraManager::GetViewTarget() const
{
    return ViewTarget.Target;
}

void APlayerCameraManager::UpdateCamera(float DeltaTime)
{
    // 만일 PendingViewTarget이 존재한다면 그로의 Transition 수행
    if (PendingViewTarget.Target != nullptr)
    {
        /* Blend 관련 switch case 분기 및 인자 설정 .. */

        /* Note) 언리얼 코드에선 BlendViewTargets 호출 X
         * 더 많은 인자와 처리를 지원하는 BlendViewInfos() 호출
         * NewPOV = ViewTarget.POV;
         * NewPOV.BlendViewInfo(PendingViewTarget.POV, BlendPct);
         */
    }

    // Fade Enabled 되었다면 Fade 처리 수행
    if (bEnableFading)
    {
        FadeTimeRemaining = FMath::Max(FadeTimeRemaining - DeltaTime, 0.0f);
        if (FadeTime > 0.0f)
        {
            FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining / FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
        }

        if (/*(bHoldFadeWhenFinished == false) && */(FadeTimeRemaining <= 0.f))
        {
            // done
            StopCameraFade();
        }
    }

    if (bAnimateVignette)
    {
        VignetteTimeRemaining = FMath::Max(VignetteTimeRemaining - DeltaTime, 0.0f);
        if (VignetteTime > 0.0f)
        {
            VignetteIntensity = VignetteStartIntensity + ((1.f - VignetteTimeRemaining / VignetteTime) * (VignetteTargetIntensity - VignetteStartIntensity));

            SetCameraVignette(VignetteIntensity, 0.8f, 0.8f);
        }

        if (VignetteTimeRemaining <= 0.0f)
        {
            VignetteIntensity = VignetteTargetIntensity;
            bAnimateVignette = false;
        }
    }
}

/**
 * @param FromAlpha          시작 알파값 (0: 완전 투명, 1: 완전 불투명)
 * @param ToAlpha            목표 알파값
 * @param InFadeTime         페이드 지속 시간 (초 단위)
 * @param InFadeColor        페이드에 사용될 색상
 * @param bInHoldWhenFinished 페이드 종료 후의 화면 유지 여부
 */
void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float InFadeTime, FLinearColor InFadeColor, bool bInHoldWhenFinished)
{
    bEnableFading = true;

    FadeColor = InFadeColor;
    FadeAlpha = FVector2D(FromAlpha, ToAlpha);
    FadeTime = InFadeTime;
    FadeTimeRemaining = InFadeTime;

    /* 오디오 효과 중략 */

    bHoldFadeWhenFinished = bInHoldWhenFinished;
}

void APlayerCameraManager::StopCameraFade()
{    
    if (bEnableFading == true)
    {
        // Make sure FadeAmount finishes at the desired value
        FadeAmount = FadeAlpha.Y;
        bEnableFading = false;
        //StopAudioFade();
    }
}

void APlayerCameraManager::SetCameraVignette(float InIntensity, float InRadius, float InSmoothness)
{
    VignetteIntensity = InIntensity;
    VignetteRadius = InRadius;
    VignetteSmoothness = InSmoothness;
}

void APlayerCameraManager::SetCameraVignetteColor(FLinearColor InColor)
{
    VignetteColor = InColor;
}

/* A로부터 B로의 ViewTarget Blend 수행
 * 실제 언리얼 코드에선 사용하지 않음
 */
FPOV APlayerCameraManager::BlendViewTargets(const FTViewTarget& A, const FTViewTarget& B, float Alpha)
{
    FPOV POV;
    POV.Location = FMath::Lerp(A.POV.Location, B.POV.Location, Alpha);
    POV.FOV = (A.POV.FOV + Alpha * (B.POV.FOV - A.POV.FOV));

    FRotator DeltaAng = (B.POV.Rotation - A.POV.Rotation).GetNormalized();
    POV.Rotation = A.POV.Rotation + DeltaAng * Alpha;

    return POV;
}

void APlayerCameraManager::StartVignetteAnimation(float FromIntensity, float ToIntensity, float Duration)
{
    bAnimateVignette = true;
    VignetteStartIntensity = FromIntensity;
    VignetteTargetIntensity = ToIntensity;
    VignetteTime = Duration;
    VignetteTimeRemaining = Duration;
}

float APlayerCameraManager::GetLetterBoxRatio()
{
    return LetterBoxWidth / LetterBoxHeight;
}
