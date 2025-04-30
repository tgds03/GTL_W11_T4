#pragma once
#include "Math/Rotator.h"
#include "Math/Vector.h"

namespace ECameraProjectionMode
{
    enum Type : int
    {
        Perspective,
        Orthographic
    };
}

/* 카메라 필요 인자 추가하시면 됩니다 */
struct FMinimalViewInfo
{
    FVector Location;
    FRotator Rotation;
    float FOV;
    /*float OrthoWidth;
    float OrthoNearClipPlane;
    float OrthoFarClipPlane;
    float AspectRatio;*/
    FMinimalViewInfo()
        : Location(0.0f, 0.0f, 0.0f)
        , Rotation(0.0f, 0.0f, 0.0f)
        , FOV(90.0f)
        /*, OrthoWidth(512.0f)
        , OrthoNearClipPlane(1.0f)
        , OrthoFarClipPlane(10000.0f)
        , AspectRatio(1.33333333f)*/
    {
    }
};

