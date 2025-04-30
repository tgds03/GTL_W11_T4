#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Camera/CameraTypes.h"

class APlayerCameraManager;

class UCameraModifier : public UObject
{
    DECLARE_CLASS(UCameraModifier, UObject)

public:
    UCameraModifier();

    /* CameraShake에서 상속받아 사용 */
    virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV); 

public:
    UWorld* GetWorld() const;
    bool IsDisabled() const { return bDisabled; }

protected:
    void EnableModifier();
    void DisableModifier(bool bImmediate);
    /** @return Returns the ideal blend alpha for this modifier. Interpolation will seek this value. */
    virtual float GetTargetAlpha();
    void UpdateAlpha(float DeltaTime);


    APlayerCameraManager* CameraOwner;

private:
    uint32 bDisabled : 1;
    uint32 bPendingDisable : 1;

    uint8 Priority;

    float AlphaInTime;
    float AlphaOutTime;
    float Alpha;
};

