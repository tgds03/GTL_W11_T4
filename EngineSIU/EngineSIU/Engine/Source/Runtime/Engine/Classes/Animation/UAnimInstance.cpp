#include "UAnimInstance.h"
#include "UObject/ObjectFactory.h"
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
        AnimStateMachine = FObjectFactory::ConstructObject<UAnimationStateMachine>(GetOuter());
        AnimStateMachine->Initialize(InOwner, "Scripts/Animation/DefaultStateMachine.lua", this);
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

    if (!OwningComponent)
    {
        return;
    }
    USkeletalMesh* Mesh = OwningComponent->GetSkeletalMesh();
    if (!Mesh)
    {
        return;
    }

    if (AnimStateMachine)
    {
        AnimStateMachine->ProcessState();
    }
    
    if(CurrentSequence)
    PreviousSequenceTime = CurrentSequence->LocalTime;

    // Current와 Target이 모두 있는 경우 애니메이션 블렌딩을 수행.
    // 블렌딩이 끝나면 Current를 Target으로 교체.
    // Target이 없는 경우 Current를 그대로 사용.
    // Current가 Looping인 경우 Looping을 유지.
    if (CurrentSequence)
    {
        CurrentSequence->TickSequence(DeltaSeconds);
    }
    if (TargetSequence)
    {
        TargetSequence->TickSequence(DeltaSeconds);
    }

    CheckAnimNotifyQueue();
    TriggerAnimNotifies();

    if (CurrentSequence && TargetSequence)
    {
        TArray<FBonePose> CurrentPose;
        CurrentSequence->GetAnimationPose(Mesh, CurrentPose);
        TArray<FBonePose> TargetPose;
        TargetSequence->GetAnimationPose(Mesh, TargetPose); 
        TArray<FBonePose> BlendedPose;
        float Alpha = ElapsedTime / BlendTime;
        for (int32 i = 0; i < CurrentPose.Num(); ++i)
        {
            FBonePose Blended;
            Blended.Location = FMath::Lerp(CurrentPose[i].Location, TargetPose[i].Location, Alpha);
            Blended.Rotation = FQuat::Slerp(CurrentPose[i].Rotation, TargetPose[i].Rotation, Alpha).GetSafeNormal();
            Blended.Scale = FMath::Lerp(CurrentPose[i].Scale, TargetPose[i].Scale, Alpha);
            BlendedPose.Add(Blended);
        }
        ElapsedTime += DeltaSeconds;
        if (ElapsedTime >= BlendTime)
        {
            ElapsedTime = 0.f;
            CurrentSequence = TargetSequence;
            TargetSequence = nullptr;
        }
        Mesh->SetBoneLocalTransforms(BlendedPose);
    }
    else if (CurrentSequence)
    {
        TArray<FBonePose> CurrentPose;
        CurrentSequence->GetAnimationPose(Mesh, CurrentPose);
        Mesh->SetBoneLocalTransforms(CurrentPose);
    }
}

void UAnimInstance::SetTargetSequence(UAnimSequence* InSequence, float InBlendTime)
{
    if (CurrentSequence == nullptr)
    {
        CurrentSequence = InSequence;
        return;
    }

    TargetSequence = InSequence;
    BlendTime = InBlendTime;
    ElapsedTime = 0.f;
}

void UAnimInstance::PlayAnimation(UAnimSequence* InSequence, bool bInLooping)
{
    InSequence->SetLooping(bInLooping);
}

void UAnimInstance::CheckAnimNotifyQueue()
{
    // 큐 초기화
    NotifyQueue.Reset();

    // 노티파이 수집
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
