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
    
    bool bIsPlaying = true;
    float BlendTime = 0.f;

    TMap<EAnimState, UAnimSequence*> AnimSequenceMap;
    EAnimState CurrentState;
    
public:
    UAnimInstance();
    virtual ~UAnimInstance() = default;

    // 매 프레임 업데이트
    void Update(float DeltaTime);
    virtual void NativeUpdateAnimation(float DeltaSeconds);

    void TriggerAnimNotifies(float DeltaSceonds);

    void CheckAnimNotifyQueue();

#pragma region Properties
    USkeletalMeshComponent* GetOwningComponent() const { return OwningComponent; }
    void SetOwningComponent(USkeletalMeshComponent* InComponent) { OwningComponent = InComponent; }
    void Initialize(USkeletalMeshComponent* InComponent, APawn* InOwner);

    // 애니메이션 재생 제어
    void PlayAnimation(UAnimSequence* InSequence, bool bInLooping = false, bool bPlayDirect = false);

    // 현재 애니메이션 접근자
    UAnimSequence* GetCurrentAnimSequence() const { return CurrentSequence; }
    void SetAnimaSequence(UAnimSequence* AnimSeq) { CurrentSequence = AnimSeq; }

    // 재생 상태 접근자
    bool IsLooping() const;
    bool IsPlaying() const { return bIsPlaying; }
#pragma endregion

    void AddAnimSequence(EAnimState InAnimState, UAnimSequence* InAnimSequence){ AnimSequenceMap.Add(InAnimState, InAnimSequence); }
    UAnimSequence* GetAnimSequence(EAnimState InAnimState){ return AnimSequenceMap[InAnimState]; }

protected:    
    void StartAnimSequence(UAnimSequence* InSequence, float InBlendingTime);
    
    // 애니메이션 상태 업데이트
    //void UpdateAnimationState(float DeltaTime);

    // 포즈 계산
    //void CalculatePose();
};
