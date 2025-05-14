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
    // 현재 프레임에 처리할 노티파이 참조 목록
    TArray<FAnimNotifyEventReference> AnimNotifies;

    TArray<FAnimNotifyEventReference> GetAnimNotifies() 
    {
        return AnimNotifies;
    }

    // 큐 초기화
    void Reset() { AnimNotifies.Empty(); }

    // 단일 노티파이 추가
    void AddAnimNotify(const FAnimNotifyEvent* Notify, const UObject* NotifySource)
    {
        if (Notify)
        {
            AnimNotifies.Add(FAnimNotifyEventReference(*Notify));
        }
    }

    // 노티파이 배열 추가 (내부 구현)
    void AddAnimNotifies(const TArray<FAnimNotifyEventReference>& NewNotifies)
    {
        for (const FAnimNotifyEventReference& NotifyRef : NewNotifies)
        {
            if (const FAnimNotifyEvent* Notify = NotifyRef.GetNotify())
            {
                AnimNotifies.Add(NotifyRef);
                
            }
        }
    }
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
