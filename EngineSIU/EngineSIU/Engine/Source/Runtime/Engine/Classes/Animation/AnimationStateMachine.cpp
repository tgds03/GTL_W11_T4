#include "AnimationStateMachine.h"

#include "AnimSequence.h"
#include "Actors/Character/Pawn.h"

void UAnimationStateMachine::Initialize(APawn* InOwner)
{
    Owner = InOwner;
}

void UAnimationStateMachine::ProcessState()
{
    if (Owner->CurrentMovementMode == EIdle)
    {
        CurrentState = AS_Idle;
    }
    else if (Owner->CurrentMovementMode == EDancing)
    {
        CurrentState = AS_Dance;
    }
    else if (Owner->CurrentMovementMode == EDie)
    {
        CurrentState = AS_Die;
    }
}

void UAnimationStateMachine::StartAnimSequence(UAnimSequence* InSequence, float InBlendingTime)
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

void UAnimationStateMachine::UpdateSequence(float DeltaTime, USkeletalMesh* InSkeletalMesh)
{
    if (AnimSequenceMap.IsEmpty())
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
    
    //바뀌면 애니메이션 체인지 -> 추가할지 바로바꿀지 결정
    if (CurrentState != PreState)
    {
        StartAnimSequence(AnimSequenceMap[CurrentState], 1.0f);

        PreState = CurrentState;
    }

    // Delegate pose calculation to the sequence
    CurrentSequence->GetAnimationPose(InSkeletalMesh, CurrentPose);

    //여기서 블렌딩로직 추가
    if (BlendSequence)
    {
        TArray<FBonePose> NewBlendPoses;
        
        BlendSequence->GetAnimationPose(InSkeletalMesh, NewBlendPoses);

        //블렌드 진행도 0~1
        float BlendAlpha = BlendSequence->LocalTime / BlendTime;
        float RemainBlendAlpha = (1 - BlendAlpha);
        
        //본 갯수가 똑같아야함
        for (int b = 0; b < CurrentPose.Num(); b++)
        {
            FBonePose& BonePose = CurrentPose[b];
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
}
