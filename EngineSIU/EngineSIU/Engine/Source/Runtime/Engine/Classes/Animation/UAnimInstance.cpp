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
