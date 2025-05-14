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
    DECLARE_CLASS(UAnimInstance, UObject)

protected:
    // 소유 컴포넌트
    USkeletalMeshComponent* OwningComponent = nullptr;
    UAnimationStateMachine* AnimStateMachine = nullptr;
   
    FAnimNotifyQueue NotifyQueue; // 노티파이 큐

    bool bIsPlaying = true;

    float PreviousSequenceTime = 0.f;

private:
    UAnimSequence* CurrentSequence = nullptr;
    UAnimSequence* TargetSequence = nullptr;
    float BlendTime = 0.f; // 블렌딩에 걸리는 시간.
    float ElapsedTime = 0.f; // 블렌딩에 걸린 시간.
    
public:
    UAnimInstance();
    virtual ~UAnimInstance() = default;

    // 매 프레임 업데이트
    void Update(float DeltaTime);
    virtual void NativeUpdateAnimation(float DeltaSeconds);

    void CheckAnimNotifyQueue();
    void TriggerAnimNotifies();

public:
    void SetTargetSequence(UAnimSequence* InSequence, float InBlendTime);

    UAnimSequence* GetCurrentSequence() const { return CurrentSequence; }

public:
#pragma region Properties
    USkeletalMeshComponent* GetOwningComponent() const { return OwningComponent; }
    void SetOwningComponent(USkeletalMeshComponent* InComponent) { OwningComponent = InComponent; }
    void Initialize(USkeletalMeshComponent* InComponent, APawn* InOwner);

    // 애니메이션 재생 제어
    void PlayAnimation(UAnimSequence* InSequence, bool bInLooping = false);

    bool IsPlaying() const { return bIsPlaying; }

    void SetIsPlaying(bool IsPlaying) { bIsPlaying = IsPlaying; }

    UAnimationStateMachine* GetAnimStateMachine() const { return AnimStateMachine; }
#pragma endregion

    UAnimInstance* DuplicateAnimInstance() const;
};

//class UViewerAnimInstance : public UAnimInstance
//{
//    DECLARE_CLASS(UViewerAnimInstance, UAnimInstance)
//
//public:
//    UViewerAnimInstance();
//    virtual ~UViewerAnimInstance() = default;
//
//private:
//    S
//}
