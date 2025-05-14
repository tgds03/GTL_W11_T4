#pragma once

#include "Define.h"
#include "Container/Map.h"
#include "Core/Math/Quat.h"

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

struct FAnimNotifyEventReference
{
    const FAnimNotifyEvent* Notify = nullptr;

    FAnimNotifyEventReference() = default;
    FAnimNotifyEventReference(const FAnimNotifyEvent& InEvent) : Notify(&InEvent) {}

    const FAnimNotifyEvent* GetNotify() const { return Notify; }
};

// 3. 노티파이 큐 클래스
class FAnimNotifyQueue
{
public:
    // 생성자
    FAnimNotifyQueue() = default;

    // 큐 초기화 (모든 노티파이 제거)
    void Reset()
    {
        AnimNotifies.Empty();
    }

    // 노티파이 추가
    void AddAnimNotify(const FAnimNotifyEvent* Notify)
    {
        if (Notify != nullptr)
        {
            AnimNotifies.Add(Notify);
        }
    }

    // 여러 노티파이 한번에 추가
    void AddAnimNotifies(const TArray<const FAnimNotifyEvent*>& Notifies)
    {
        for (const FAnimNotifyEvent* Notify : Notifies)
        {
            AddAnimNotify(Notify);
        }
    }

    // 큐에 있는 노티파이 개수 반환
    int32 Num() const
    {
        return AnimNotifies.Num();
    }

    // 큐가 비어있는지 확인
    bool IsEmpty() const
    {
        return AnimNotifies.Num() == 0;
    }

    // 현재 프레임에 처리할 노티파이 목록
    TArray<const FAnimNotifyEvent*> AnimNotifies;
};

struct FFrameRate
{
    /**
     * Default construction to a frame rate of 60000 frames per second (0.0166 ms)
     */
    FFrameRate()
        : Numerator(60000), Denominator(1)
    {
    }

    FFrameRate(uint32 InNumerator, uint32 InDenominator)
        : Numerator(InNumerator), Denominator(InDenominator)
    {
    }

    /** IMPORTANT: If you change the struct data, ensure that you also update the version in NoExportTypes.h  */

    /**
     * The numerator of the framerate represented as a number of frames per second (e.g. 60 for 60 fps)
     */
    int32 Numerator;

    /**
     * The denominator of the framerate represented as a number of frames per second (e.g. 1 for 60 fps)
     */
    int32 Denominator   ;
};

struct FAnimationCurveData
{
    TMap<FName, TArray<float>> FloatCurves;  // 이름별 플로트 커브 데이터
    TMap<FName, TArray<bool>> BoolCurves;    // 이름별 불린 커브 데이터
};
