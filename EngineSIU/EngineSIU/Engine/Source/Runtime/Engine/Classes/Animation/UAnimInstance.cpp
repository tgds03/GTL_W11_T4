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

void UAnimInstance::StartAnimSequence(UAnimSequence* InSequence, float InBlendingTime)
{
    if (!CurrentSequence)
    {
        CurrentSequence = InSequence;
        CurrentSequence->BeginSequence();
        return;
    }

    BlendSequence = InSequence;
    BlendTime = InBlendingTime;
    BlendSequence->BeginSequence();
}

void UAnimInstance::Update(float DeltaTime)
{
    if (!bIsPlaying)
    {
        return;
    }

    if (CurrentSequence)
    {
        CurrentSequence->TickSequence(DeltaTime);
    }

    if (BlendSequence)
    {
        BlendSequence->TickSequence(DeltaTime);
    }
    
    NativeUpdateAnimation(DeltaTime);
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (!OwningComponent || AnimSequenceMap.IsEmpty())
    {
        return;
    }

    if (AnimStateMachine)
    {
        AnimStateMachine->ProcessState();
    }

    if (AnimSequenceMap.IsEmpty())
    {
        return;
    }

    //바뀌면 애니메이션 체인지 -> 추가할지 바로바꿀지 결정
    if (CurrentState != AnimStateMachine->CurrentState)
    {
        StartAnimSequence(AnimSequenceMap[AnimStateMachine->CurrentState], 1.0f);

        CurrentState = AnimStateMachine->CurrentState;
    }

    // Delegate pose calculation to the sequence
    TArray<FBonePose> NewLocalPoses;
    USkeletalMesh* Mesh = OwningComponent->GetSkeletalMesh();
    CurrentSequence->GetAnimationPose(Mesh, NewLocalPoses);

    //여기서 블렌딩로직 추가
    if (BlendSequence)
    {
        TArray<FBonePose> NewBlendPoses;
        
        BlendSequence->GetAnimationPose(Mesh, NewBlendPoses);

        //블렌드 진행도 0~1
        float BlendAlpha = BlendSequence->LocalTime / BlendTime;
        float RemainBlendAlpha = (1 - BlendAlpha);
        
        //본 갯수가 똑같아야함
        for (int b = 0; b < NewLocalPoses.Num(); b++)
        {
            FBonePose& BonePose = NewLocalPoses[b];
            FBonePose& BlendBonePose = NewBlendPoses[b];

            BonePose.Location = (BonePose.Location * RemainBlendAlpha) + (BlendBonePose.Location * BlendAlpha);
            
            BonePose.Rotation = FQuat::Slerp(BonePose.Rotation, BlendBonePose.Rotation, BlendAlpha).GetNormalized();

            BonePose.Scale = (BonePose.Scale * RemainBlendAlpha) + (BlendBonePose.Scale * BlendAlpha);
        }

        // 현재 애니메이션이 끝나거나 블렌드시간이 지나면 애니메이션 교체
        if (CurrentSequence->LocalTime > CurrentSequence->GetUnScaledPlayLength() || BlendSequence->LocalTime > BlendTime)
        {
            CurrentSequence = BlendSequence;
            BlendSequence = nullptr;
            BlendTime = 0.f;
        }
    }
    //여기서 만든 NewBlendPoses로 Blend해야함

    // Apply poses and update global transforms
    Mesh->SetBoneLocalTransforms(NewLocalPoses);
}

void UAnimInstance::PlayAnimation(UAnimSequence* InSequence, bool bInLooping, bool bPlayDirect)
{
    InSequence->SetLooping(bInLooping);

    if (bPlayDirect)
    {
        StartAnimSequence(InSequence, 0.0f);
    }
    else
    {
        WaitSequences.Enqueue(InSequence);
    }
}

bool UAnimInstance::IsLooping() const
{
    if (CurrentSequence)
    {
        return CurrentSequence->IsLooping();
    }

    return false;
}
