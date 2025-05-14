#include "UAnimInstance.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"
#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "AnimationStateMachine.h"
#include <memory>

UAnimInstance::UAnimInstance()
{
}

void UAnimInstance::Initialize(USkeletalMeshComponent* InComponent, APawn* InOwner)
{
    OwningComponent = InComponent;
    
    if (!AnimStateMachine)
    {
        AnimStateMachine = std::make_shared<UAnimationStateMachine>();
        AnimStateMachine->Initialize(InOwner);
    }
}

void UAnimInstance::Update(float DeltaTime)
{
    if (!bIsPlaying)
    {
        return;
    }
    
    NativeUpdateAnimation(DeltaTime);
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (!OwningComponent || !AnimStateMachine)
    {
        return;
    }
    
    USkeletalMesh* Mesh = OwningComponent->GetSkeletalMesh();
    
    AnimStateMachine->ProcessState();
    AnimStateMachine->UpdateSequence(DeltaSeconds, Mesh);

    TArray<FBonePose> NewLocalPoses = AnimStateMachine->GetCurrentPose();
    
    if (NewLocalPoses.Num() > 0)
    {
        // Apply poses and update global transforms
        Mesh->SetBoneLocalTransforms(NewLocalPoses);
    }
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSceonds)
{
}

void UAnimInstance::CheckAnimNotifyQueue()
{
    // 큐 초기화
    NotifyQueue.Reset();

    // 노티파이 수집
    if (CurrentSequence && CurrentSequence->Notifies.Num() > 0)
    {

        float SequenceLength = CurrentSequence->GetUnScaledPlayLength();
        if (SequenceLength <= 0.0f)
            return;

        float NormalizedPrevTime = PreviousLocalTime / SequenceLength;
        float NormalizedCurrTime = CurrentSequence->LocalTime / SequenceLength;

        bool bLoopedThisFrame = NormalizedCurrTime < NormalizedPrevTime;

        for (const FAnimNotifyEvent& Notify : CurrentSequence->Notifies)
        {
            bool bShouldTrigger = false;

            if (bLoopedThisFrame)
            {
                // 루핑 케이스: 두 부분 확인 (이전~1.0 또는 0.0~현재)
                bShouldTrigger = (Notify.TriggerTime > NormalizedPrevTime && Notify.TriggerTime <= 1.0f) ||
                    (Notify.TriggerTime >= 0.0f && Notify.TriggerTime <= NormalizedCurrTime);
            }
            else
            {
                // 일반 케이스: 두 시간 사이 확인
                bShouldTrigger = (Notify.TriggerTime > NormalizedPrevTime &&
                    Notify.TriggerTime <= NormalizedCurrTime);
            }

            if (bShouldTrigger)
            {
                NotifyQueue.AddAnimNotify(&Notify);
            }
        }
    }
}

void UAnimInstance::TriggerAnimNotifies()
{
}


void UAnimInstance::PlayAnimation(UAnimSequence* InSequence, bool bInLooping, bool bPlayDirect)
{
    InSequence->SetLooping(bInLooping);

    if (bPlayDirect)
    {
        AnimStateMachine->StartAnimSequence(InSequence, 0.0f);
    }
    else
    {
        WaitSequences.Enqueue(InSequence);
    }
}

bool UAnimInstance::IsLooping() const
{
    if (AnimStateMachine)
    {
        if (AnimStateMachine->CurrentSequence)
        {
            return AnimStateMachine->CurrentSequence->IsLooping();
        }
    }

    return false;
}
