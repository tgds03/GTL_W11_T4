#pragma once

#include "AnimationStateMachine.h"
#include "AnimSequence.h"
#include "UAnimDataModel.h"
#include "Container/Queue.h"
#include "Launch/SkeletalDefine.h"

class UAnimationStateMachine;
class USkeletalMeshComponent;
class UAnimSequence;

class UAnimInstance : public UObject
{
protected:
    // 소유 컴포넌트
    USkeletalMeshComponent* OwningComponent = nullptr;

    UAnimSequence* CurrentSequence = nullptr;
    UAnimSequence* BlendSequence = nullptr;

    std::shared_ptr<UAnimationStateMachine> AnimStateMachine = nullptr;

    TQueue<UAnimSequence*> WaitSequences;
    
    // 재생 상태
    bool bIsPlaying;
    float CurrentTime;
    float PlayRate;
    
    float BlendTime;
    float BlendCurrentTime;
    
    TMap<EAnimState, UAnimSequence*> AnimSequenceMap;
    EAnimState CurrentState;
    
public:
    UAnimInstance();
    virtual ~UAnimInstance() = default;

    // 컴포넌트와 연결
    void Initialize(USkeletalMeshComponent* InComponent, APawn* InOwner);
    
    // 매 프레임 업데이트
    void Update(float DeltaTime);
    void ChangeAnimation(UAnimSequence* NewAnim, float InBlendingTime);

    // 애니메이션 재생 제어
    void PlayAnimation(UAnimSequence* InSequence, bool bInLooping = false, bool bPlayDirect = false);

    // 현재 애니메이션 접근자
    UAnimSequence* GetCurrentAnimSequence() const { return CurrentSequence; }
    void SetAnimaSequence(UAnimSequence* AnimSeq) { CurrentSequence = AnimSeq; }
    // 현재 포즈 접근자
   // const TArray<FTransform>& GetCurrentPose() const { return CurrentPose; }

    // 재생 상태 접근자
    bool IsLooping() const;
    bool IsPlaying() const { return bIsPlaying; }
    float GetCurrentTime() const { return CurrentTime; }
    void SetCurrentTime(float InTime);

    // 재생 속도 설정
    void SetPlayRate(float InRate) { PlayRate = InRate; }
    float GetPlayRate() const { return PlayRate; }

    void GetBoneTransforms(TArray<FBonePose>& OutTransforms);

    void AddAnimSequence(EAnimState InAnimState, UAnimSequence* InAnimSequence){ AnimSequenceMap.Add(InAnimState, InAnimSequence); }
    UAnimSequence* GetAnimSequence(EAnimState InAnimState){ return AnimSequenceMap[InAnimState]; }

protected:
    // 애니메이션 노티파이 처리
    void ProcessNotifies(float PreviousTime, float CurrentTime);

    void ProcessState();
    
    void StartAnimSequence(UAnimSequence* InSequence);
    
    // 애니메이션 상태 업데이트
    void UpdateAnimationState(float DeltaTime);

    // 포즈 계산
    void CalculatePose();
};
