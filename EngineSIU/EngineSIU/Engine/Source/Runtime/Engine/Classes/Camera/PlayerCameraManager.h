#pragma once
#include "GameFramework/Actor.h"

class UCameraComponent;
class APlayerController;
class UCameraModifier;

// enum EViewTargetBlendFunction : int
// struct FViewTargetTransitionParams

struct FTViewTarget
{
    AActor* Target;
    FMinimalViewInfo POV;
public:

    void SetNewTarget(AActor* NewTarget);
    AActor* GetTargetActor() const;

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
protected:

    FMinimalViewInfo BlendViewTargets(const FTViewTarget& A, const FTViewTarget& B, float Alpha);

protected:
    TArray<UCameraModifier*> ModifierList;

public:
    UCameraComponent* TargetCamera = nullptr;
    
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

    uint32 bEnableFading : 1;
    uint32 bHoldFadeWhenFinished : 1; /* true일 경우 페이드 종료 상태 유지 */
};

