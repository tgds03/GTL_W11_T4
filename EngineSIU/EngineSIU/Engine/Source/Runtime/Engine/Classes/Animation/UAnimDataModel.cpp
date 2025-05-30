#include "UAnimDataModel.h"
// Animation/AnimDataModel.cpp 파일에 정의하세요

UAnimDataModel::UAnimDataModel()
    : PlayLength(0.0f)
    , FrameRate(30, 1)
    , NumberOfFrames(0)
    , NumberOfKeys(0)
{
}

void UAnimDataModel::AddBoneTrack(const FBoneAnimationTrack& Track)
{
    BoneAnimationTracks.Add(Track);
}

const FBoneAnimationTrack* UAnimDataModel::FindBoneTrack(const FName& BoneName) const
{
    for (const FBoneAnimationTrack& Track : BoneAnimationTracks)
    {
        if (Track.Name == BoneName)
        {
            return &Track;
        }
    }

    return nullptr;
}

void UAnimDataModel::SetPlayLength(float InLength)
{
    PlayLength = InLength;
}

void UAnimDataModel::SetFrameRate(const FFrameRate& InRate)
{
    FrameRate = InRate;
}

void UAnimDataModel::SetNumberOfFrames(int32 InFrames)
{
    NumberOfFrames = InFrames;
}


void UAnimDataModel::SetNumberOfKeys(int32 InKeys)
{
    NumberOfKeys = InKeys;
}

// (LoadFromFBX 및 다른 메서드 구현은 그대로 유지)
