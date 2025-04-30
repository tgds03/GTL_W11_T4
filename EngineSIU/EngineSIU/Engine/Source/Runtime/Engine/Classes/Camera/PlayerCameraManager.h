#pragma once
#include "CameraTypes.h"
#include "GameFramework/Actor.h"

class UCameraComponent;
class UCameraModifier_CameraShake;
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

    APlayerController* PCOwner;
    
public:
    APlayerCameraManager();

    virtual void InitializeFor(APlayerController* PC);

    AActor* GetViewTarget() const;
    
    void UpdateCamera(float DeltaTime);

    /* Fade IN / OUT */
    void StartCameraFade(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bHoldWhenFinished = false);

    void StopCameraFade();

    void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV);
    
protected:
    virtual void DoUpdateCamera(float DeltaTime);

    FMinimalViewInfo BlendViewTargets(const FTViewTarget& A, const FTViewTarget& B, float Alpha);
    virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime);

    TArray<UCameraModifier*> ModifierList;

    UCameraModifier_CameraShake* CachedCameraShakeMod;

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

    uint32 bEnableFading : 1;
    uint32 bHoldFadeWhenFinished : 1; /* true일 경우 페이드 종료 상태 유지 */
};

