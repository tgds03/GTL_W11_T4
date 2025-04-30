#pragma once
#include "Core/HAL/PlatformType.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

namespace EEndPlayReason
{
enum Type : uint8
{
    /** 명시적인 삭제가 일어났을 때, Destroy()등 */
    Destroyed,
    /** World가 바뀌었을 때 */
    WorldTransition,
    /** 프로그램을 종료했을 때 */
    Quit,
};
}


struct FMinimalViewInfo //CameraInfo
{
    FVector Location;
    FRotator Rotation;
    float FOV;
    FVector CameraToViewTarget;
    
    FMinimalViewInfo()
        : Location(), Rotation(), FOV(90.0f)
    {
    }

    FMinimalViewInfo(FVector InLocation, FRotator InRotation, float InFOV)
        : Location(InLocation), Rotation(InRotation), FOV(InFOV)
    {
    }

    // Serializer.
    friend FArchive& operator<<(FArchive& Ar, FMinimalViewInfo& POV)
    {
        return Ar << POV.Location << POV.Rotation << POV.FOV;
    }
};
