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
    virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) { return false; }
    bool IsDisabled() const { return bDisabled; }
protected:
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

