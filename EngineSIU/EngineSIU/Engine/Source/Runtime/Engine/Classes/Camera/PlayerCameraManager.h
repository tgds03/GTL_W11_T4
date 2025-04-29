#pragma once
#include "GameFramework/Actor.h"

class APlayerController;
class UCameraModifier;

// enum EViewTargetBlendFunction : int
// struct FViewTargetTransitionParams

struct FTViewTarget
{
    AActor* Target;

public:

    void SetNewTarget(AActor* NewTarget);

    bool Equal(const FTViewTarget& OtherTarget) const;

    FTViewTarget()
        : Target(nullptr)
    {
    }

    /** Make sure ViewTarget is valid */
    void CheckViewTarget(APlayerController* OwningController);
};



class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)
public:
    APlayerCameraManager();

protected:
    TArray<UCameraModifier*> ModifierList;

private:
    FLinearColor FadeColor;
    float FadeAmount;

    /** Current camera fade alpha range, where X = starting alpha and Y = final alpha (when bEnableFading == true) */
    FVector2D FadeAlpha;

    /** Total duration of the camera fade (when bEnableFading == true) */
    float FadeTime;

    /** Time remaining in camera fade (when bEnableFading == true) */
    float FadeTimeRemaining;

    FName CameraStyle;

    FTViewTarget ViewTarget;

};

