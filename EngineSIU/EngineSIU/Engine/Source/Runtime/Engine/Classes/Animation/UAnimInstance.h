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
    
    float PreviousLocalTime = 0.0f;   // 이전 프레임의 시간
    FAnimNotifyQueue NotifyQueue;     // 노티파이 큐

    // 노티파이 처리 함수
    void CheckAnimNotifyQueue();
    void TriggerAnimNotifies();


    bool bIsPlaying = true;
    
public:
    UAnimInstance();
    virtual ~UAnimInstance() = default;

    // 매 프레임 업데이트
    void Update(float DeltaTime);
    virtual void NativeUpdateAnimation(float DeltaSeconds);

    void TriggerAnimNotifies(float DeltaSceonds);

#pragma region Properties
    USkeletalMeshComponent* GetOwningComponent() const { return OwningComponent; }
    void SetOwningComponent(USkeletalMeshComponent* InComponent) { OwningComponent = InComponent; }
    void Initialize(USkeletalMeshComponent* InComponent, APawn* InOwner);

    // 애니메이션 재생 제어
    void PlayAnimation(UAnimSequence* InSequence, bool bInLooping = false, bool bPlayDirect = false);

    // 재생 상태 접근자
    bool IsLooping() const;
    bool IsPlaying() const { return bIsPlaying; }

    UAnimationStateMachine* GetAnimStateMachine() const { return AnimStateMachine.get(); }
    
#pragma endregion
};
