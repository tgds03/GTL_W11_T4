#pragma once
#include "CameraTypes.h"
#include "GameFramework/Actor.h"

class UCameraComponent;
class UCameraModifier_CameraShake;
class APlayerController;
class UCameraModifier;

DECLARE_MULTICAST_DELEGATE(FOnBlendComplete)

enum EViewTargetBlendOrder : int
{
    VTBlendOrder_Base,
    VTBlendOrder_Override
};

enum EViewTargetBlendFunction : int
{
    /** Camera does a simple linear interpolation. */
    VTBlend_Linear,
    /** Camera has a slight ease in and ease out, but amount of ease cannot be tweaked. */
    VTBlend_Cubic,
    /** Camera immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by BlendExp. */
    VTBlend_EaseIn,
    /** Camera smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by BlendExp. */
    VTBlend_EaseOut,
    /** Camera smoothly accelerates and decelerates.  Ease amount controlled by BlendExp. */
    VTBlend_EaseInOut,
    /** The game's camera system has already performed the blending. Engine should not blend at all */
    VTBlend_PreBlended,
    VTBlend_MAX,
};

struct FTViewTarget
{
    AActor* Target; 
    FMinimalViewInfo POV;

public:
    bool Equal(const FTViewTarget& OtherTarget) const;

    FTViewTarget()
        : Target(nullptr)
    {
    }

    void CheckViewTarget(APlayerController* OwningController);

    void SetNewTarget(AActor* NewTarget);
    AActor* GetTargetActor() const;
};

struct FViewTargetTransitionParams
{
public:
    float BlendTime;

    EViewTargetBlendFunction BlendFunction;

    float BlendExp;

    uint32 bLockOutgoing:1;

    FViewTargetTransitionParams()
        : BlendTime(0.f)
        , BlendFunction(VTBlend_Cubic)
        , BlendExp(2.f)
        , bLockOutgoing(false)
    {}

    /** For a given linear blend value (blend percentage), return the final blend alpha with the requested function applied */
    float GetBlendAlpha(const float& TimePct) const
    {
        switch (BlendFunction)
        {
        case VTBlend_Linear: return FMath::Lerp(0.f, 1.f, TimePct); 
        case VTBlend_Cubic:	return FMath::CubicInterp(0.f, 0.f, 1.f, 0.f, TimePct); 
        case VTBlend_EaseInOut: return FMath::InterpEaseInOut(0.f, 1.f, TimePct, BlendExp); 
        case VTBlend_EaseIn: return FMath::Lerp(0.f, 1.f, FMath::Pow(TimePct, BlendExp)); 
        case VTBlend_EaseOut: return FMath::Lerp(0.f, 1.f, FMath::Pow(TimePct, (FMath::IsNearlyZero(BlendExp) ? 1.f : (1.f / BlendExp))));
        default:
            break;
        }

        return 1.f;
    }
};

struct FCameraCacheEntry
{
public:

    /** World time this entry was created. */
    float TimeStamp;

    /** Camera POV to cache. */
    FMinimalViewInfo POV;

    FCameraCacheEntry()
        : TimeStamp(0.f)
    {}
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
    void AddCachedPPBlend(float BlendWeight, EViewTargetBlendOrder BlendOrder);

    void AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams=FViewTargetTransitionParams());
    
    void SetViewTarget(class AActor* NewTarget, struct FViewTargetTransitionParams TransitionParams);

    virtual void SetCameraCachePOV(const FMinimalViewInfo& InPOV);
    virtual void SetLastFrameCameraCachePOV(const FMinimalViewInfo& InPOV);
    virtual const FMinimalViewInfo& GetCameraCacheView() const;
    virtual const FMinimalViewInfo& GetLastFrameCameraCacheView() const;

    void FillCameraCache(const FMinimalViewInfo& NewInfo);

    float GetCameraCacheTime() const { return CameraCachePrivate.TimeStamp; }

    float GetLastFrameCameraCacheTime() const { return LastFrameCameraCachePrivate.TimeStamp; }

    /** Get value of CameraCachePrivate.Time  */
    void SetCameraCacheTime(float InTime) { CameraCachePrivate.TimeStamp = InTime; }

    /** Get value of LastFrameCameraCachePrivate.Time  */
    void SetLastFrameCameraCacheTime(float InTime) { LastFrameCameraCachePrivate.TimeStamp = InTime; }
    
protected:
    virtual void DoUpdateCamera(float DeltaTime);

    virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime);
    
    FMinimalViewInfo BlendViewTargets(const FTViewTarget& A, const FTViewTarget& B, float Alpha);

    FMinimalViewInfo LastFrameFOV;

    TArray<UCameraModifier*> ModifierList;

    UCameraModifier_CameraShake* CachedCameraShakeMod;

private:
    struct FCameraCacheEntry CameraCachePrivate;

    struct FCameraCacheEntry LastFrameCameraCachePrivate;
    
public:
    FTViewTarget ViewTarget;

    FTViewTarget PendingViewTarget;

    float BlendTimeToGo;

    FViewTargetTransitionParams BlendParams;
    
    FLinearColor FadeColor;

    float FadeAmount;

    FVector2D FadeAlpha;

    float FadeTime;

    float FadeTimeRemaining;

    FName CameraStyle;

    float DefaultFOV;
    float DefaultAspectRatio;
    uint32 bDefaultConstrainAspectRatio : 1;

    mutable FOnBlendComplete OnBlendCompleteEvent;

  	/** Minimum view pitch, in degrees. */
    float ViewPitchMin;
    /** Maximum view pitch, in degrees. */
    float ViewPitchMax;
    /** Minimum view yaw, in degrees. */
    float ViewYawMin;
    /** Maximum view yaw, in degrees. */
    float ViewYawMax;
    /** Minimum view roll, in degrees. */
    float ViewRollMin;
    /** Maximum view roll, in degrees. */
    float ViewRollMax;
    
    uint32 bEnableFading : 1;
    uint32 bHoldFadeWhenFinished : 1; /* true일 경우 페이드 종료 상태 유지 */
};

