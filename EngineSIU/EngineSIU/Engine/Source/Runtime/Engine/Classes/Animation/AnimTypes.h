#pragma once

#include "Define.h"

struct FRawAnimSequenceTrack
{
    TArray<FVector> PosKeys;
    TArray<FQuat> RotKeys;
    TArray<FVector> ScaleKeys;
};

struct FBoneAnimationTrack
{
    FRawAnimSequenceTrack InternalTrackData;
    int32 BoneTreeIndex = INDEX_NONE;
    FName Name;
};

struct FAnimNotifyEvent 
{
    float TriggerTime;
    float Duration;
    FName NotifyName;
};
