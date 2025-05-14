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
    
    if(AnimStateMachine->GetCurrentAnimSequence())
    PreviousSequenceTime = AnimStateMachine->GetCurrentAnimSequence()->LocalTime;

    AnimStateMachine->ProcessState();
    AnimStateMachine->UpdateSequence(DeltaSeconds, Mesh);

    CheckAnimNotifyQueue();
    TriggerAnimNotifies();

    TArray<FBonePose> NewLocalPoses = AnimStateMachine->GetCurrentPose();
    
    if (NewLocalPoses.Num() > 0)
    {
        // Apply poses and update global transforms
        Mesh->SetBoneLocalTransforms(NewLocalPoses);
    }
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

void UAnimInstance::CheckAnimNotifyQueue()
{
    // 큐 초기화
    NotifyQueue.Reset();

    // 노티파이 수집
    UAnimSequence* CurrentSequence = GetAnimStateMachine()->GetCurrentAnimSequence();
    if (!CurrentSequence || CurrentSequence->Notifies.Num() == 0)
        return;

    // 재생 방향 확인 (중요!)
    float PlayRate = CurrentSequence->GetRateScale();
    bool bIsPlayingBackwards = PlayRate < 0.0f;

    // 시간 정규화
    float SequenceLength = CurrentSequence->GetUnScaledPlayLength();
    if (SequenceLength <= 0.0f)
        return;

    float NormalizedPrevTime = PreviousSequenceTime / SequenceLength;
    float NormalizedCurrTime = CurrentSequence->LocalTime / SequenceLength;

    // 루핑 확인 (방향에 따라 다름)
    bool bLoopedThisFrame = false;
    if (!bIsPlayingBackwards)
    {
        // 정방향 재생 시 루핑: 현재 < 이전
        bLoopedThisFrame = NormalizedCurrTime < NormalizedPrevTime;
    }
    else
    {
        // 역방향 재생 시 루핑: 현재 > 이전
        bLoopedThisFrame = NormalizedCurrTime > NormalizedPrevTime;
    }

    // 각 노티파이 검사
    for (const FAnimNotifyEvent& Notify : CurrentSequence->Notifies)
    {
        bool bShouldTrigger = false;

        if (!bIsPlayingBackwards)
        {
            // 정방향 재생
            if (bLoopedThisFrame)
            {
                // 루핑 케이스: 두 부분 확인 (이전~1.0 또는 0.0~현재)
                bShouldTrigger = (Notify.TriggerTime > NormalizedPrevTime && Notify.TriggerTime <= 1.0f) ||
                    (Notify.TriggerTime >= 0.0f && Notify.TriggerTime <= NormalizedCurrTime);
            }
            else
            {
                // 일반 케이스: 이전 < 트리거 <= 현재
                bShouldTrigger = (Notify.TriggerTime > NormalizedPrevTime &&
                    Notify.TriggerTime <= NormalizedCurrTime);
            }
        }
        else
        {
            // 역방향 재생 (조건 반전)
            if (bLoopedThisFrame)
            {
                // 루핑 케이스: 두 부분 확인 (이전~0.0 또는 1.0~현재)
                bShouldTrigger = (Notify.TriggerTime < NormalizedPrevTime && Notify.TriggerTime >= 0.0f) ||
                    (Notify.TriggerTime <= 1.0f && Notify.TriggerTime >= NormalizedCurrTime);
            }
            else
            {
                // 일반 케이스: 이전 > 트리거 >= 현재
                bShouldTrigger = (Notify.TriggerTime < NormalizedPrevTime &&
                    Notify.TriggerTime >= NormalizedCurrTime);
            }
        }

        if (bShouldTrigger)
        {
            NotifyQueue.AddAnimNotify(&Notify);
        }
    }
}
void UAnimInstance::TriggerAnimNotifies()
{
    if (!OwningComponent)
        return;

    // 수집된 모든 노티파이 처리
    for (const FAnimNotifyEvent* Notify : NotifyQueue.AnimNotifies)
    {
        if (Notify)
        {
            OwningComponent->HandleAnimNotify(Notify);
        }
    }
}
