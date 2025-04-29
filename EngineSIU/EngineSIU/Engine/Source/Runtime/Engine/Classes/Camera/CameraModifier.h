#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class APlayerCameraManager;

class UCameraModifier : public UObject
{
    DECLARE_CLASS(UCameraModifier, UObject)

public:
    UCameraModifier();

protected:
    virtual float GetTargetAlpha();


    APlayerCameraManager* CameraOwner;
};

