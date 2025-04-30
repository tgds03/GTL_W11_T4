#pragma once
#include "GameFramework/Actor.h"

class APlayerController;
class UCameraModifier;

// enum EViewTargetBlendFunction : int
// struct FViewTargetTransitionParams

struct FTViewTarget
{
    AActor* Target;
    FPOV POV;
public:

    void SetNewTarget(AActor* NewTarget);

    bool Equal(const FTViewTarget& OtherTarget) const;

    FTViewTarget()
        : Target(nullptr)
    {
    }

    void CheckViewTarget(APlayerController* OwningController);
};



class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)
public:
    APlayerCameraManager();

    AActor* GetViewTarget() const;
    
    void UpdateCamera(float DeltaTime);

    /* Fade IN / OUT */
    void StartCameraFade(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bHoldWhenFinished = false);
    void StopCameraFade();

    void SetCameraVignette(float InIntensity, float InRadius, float InSmoothness);
    void SetCameraVignetteColor(FLinearColor InColor);
    void StartVignetteAnimation(float FromIntensity, float ToIntensity, float Duration);
    
    float GetLetterBoxRatio();
protected:


    FPOV BlendViewTargets(const FTViewTarget& A, const FTViewTarget& B, float Alpha);


protected:
    TArray<UCameraModifier*> ModifierList;

public:
    FTViewTarget ViewTarget;

    FTViewTarget PendingViewTarget;

    float BlendTimeToGo;

    // FViewTargetTransitionParams BlendParams;
    
    FLinearColor FadeColor;

    float FadeAmount;

    FVector2D FadeAlpha;

    float FadeTime;

    float FadeTimeRemaining;

    FName CameraStyle;

    // [TEMP] Vignette factor
    FVector2D VignetteCenter;

    FLinearColor VignetteColor;

    float VignetteRadius;

    float VignetteIntensity;

    float VignetteSmoothness;

    float VignetteTime;

    float VignetteTimeRemaining;

    float VignetteStartIntensity;

    float VignetteTargetIntensity;

    // [TEMP] LetterBox factor;
    float LetterBoxWidth;

    float LetterBoxHeight;

    uint32 bEnableFading : 1;
    uint32 bAnimateVignette : 1;
    uint32 bHoldFadeWhenFinished : 1; /* true일 경우 페이드 종료 상태 유지 */
};

