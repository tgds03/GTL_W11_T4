#pragma once
#include "Animation/AnimTypes.h"
#include "Runtime/CoreUObject/UObject/Object.h"

class UAnimDataModel : public UObject
{
private:
    TArray<FBoneAnimationTrack> BoneAnimationTracks;

    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;

public:
    // 생성자
    UAnimDataModel();

    // 본 애니메이션 트랙 접근자
    const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;

    // 본 트랙 추가
    void AddBoneTrack(const FBoneAnimationTrack& Track);

    // 특정 본의 트랙 찾기
    const FBoneAnimationTrack* FindBoneTrack(const FName& BoneName) const;

    // 애니메이션 길이 접근자
    float GetPlayLength() const;
    void SetPlayLength(float InLength);

    // 프레임 레이트 접근자
    FFrameRate GetFrameRate() const;
    void SetFrameRate(const FFrameRate& InRate);

    // 프레임 수 접근자
    int32 GetNumberOfFrames() const;
    void SetNumberOfFrames(int32 InFrames);

    // 키 수 접근자
    int32 GetNumberOfKeys() const;
    void SetNumberOfKeys(int32 InKeys);

    // FBX 파일에서 데이터 로드
    bool LoadFromFBX(const FString& FilePath);

private:
    // FBX에서 본 트랙 추출 (내부 헬퍼 함수)
    void ExtractBoneTracksFromFBX(
        FbxNode* InRootNode,
        FbxAnimLayer* InAnimLayer,
        FbxTime InStartTime,
        FbxTime InEndTime,
        FbxTime::EMode InTimeMode
    );

    // 단일 FBX 노드에서 본 트랙 추출 (재귀적으로 호출됨)
    void ExtractBoneTrackFromNode(
        FbxNode* InNode,
        FbxAnimLayer* InAnimLayer,
        FbxTime InStartTime,
        FbxTime InEndTime,
        FbxTime::EMode InTimeMode
    );
};
