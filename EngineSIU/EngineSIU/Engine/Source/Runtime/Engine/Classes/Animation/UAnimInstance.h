#pragma once

#include "AnimationStateMachine.h"
#include "AnimSequence.h"
#include "UAnimDataModel.h"
#include "Container/Queue.h"

class UAnimationStateMachine;
class USkeletalMeshComponent;
class UAnimSequence;

class UAnimInstance : public UObject
{
protected:
    // 소유 컴포넌트
    USkeletalMeshComponent* OwningComponent = nullptr;

    std::shared_ptr<UAnimationStateMachine> AnimStateMachine = nullptr;

    TQueue<UAnimSequence*> WaitSequences;
   
    FAnimNotifyQueue NotifyQueue;     // 노티파이 큐

    bool bIsPlaying = true;

    float PreviousSequenceTime = 0.f;

    
public:
    UAnimInstance();
    virtual ~UAnimInstance() = default;

    // 매 프레임 업데이트
    void Update(float DeltaTime);
    virtual void NativeUpdateAnimation(float DeltaSeconds);

    void CheckAnimNotifyQueue();
    void TriggerAnimNotifies();

#pragma region Properties
    USkeletalMeshComponent* GetOwningComponent() const { return OwningComponent; }
    void SetOwningComponent(USkeletalMeshComponent* InComponent) { OwningComponent = InComponent; }
    void Initialize(USkeletalMeshComponent* InComponent, APawn* InOwner);

    // 애니메이션 재생 제어
    void PlayAnimation(UAnimSequence* InSequence, bool bInLooping = false, bool bPlayDirect = false);

    // 재생 상태 접근자
    bool IsLooping() const;

    bool IsPlaying() const { return bIsPlaying; }

    void SetIsPlaying(bool IsPlaying) { bIsPlaying = IsPlaying; }

    UAnimationStateMachine* GetAnimStateMachine() const { return AnimStateMachine.get(); }
    
#pragma endregion
};
