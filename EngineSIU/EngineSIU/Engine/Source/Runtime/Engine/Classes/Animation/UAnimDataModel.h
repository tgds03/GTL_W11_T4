#pragma once
#include "Animation/AnimTypes.h"
#include "Runtime/CoreUObject/UObject/Object.h"
#include "UObject/ObjectMacros.h"
class UAnimDataModel : public UObject
{
    DECLARE_CLASS(UAnimDataModel, UObject);
private:
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    FFrameRate FrameRate; // 어떻게 사용 될지
    int32 NumberOfFrames;
    int32 NumberOfKeys;

public:
    // 생성자
    UAnimDataModel();
    virtual ~UAnimDataModel() = default;

    // 본 트랙 추가
    void AddBoneTrack(const FBoneAnimationTrack& Track);

    // 특정 본의 트랙 찾기
    const FBoneAnimationTrack* FindBoneTrack(const FName& BoneName) const;

    void SetPlayLength(float InLength);
    void SetFrameRate(const FFrameRate& InRate);
    void SetNumberOfFrames(int32 InFrames);
    void SetNumberOfKeys(int32 InKeys);

    const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const { return BoneAnimationTracks; }
    float GetPlayLength() const { return PlayLength; }
    FFrameRate GetFrameRate() const { return FrameRate; }
    int32 GetNumberOfFrames() const { return NumberOfFrames; }
    int32 GetNumberOfKeys() const { return NumberOfKeys; }
};
