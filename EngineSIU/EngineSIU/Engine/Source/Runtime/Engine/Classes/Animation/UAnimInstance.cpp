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
        AnimStateMachine->Initialize(InOwner, "Scripts/DefaultStateMachine.lua", this);
    }
}

void UAnimInstance::Update(float DeltaTime)
{
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
    else if (TargetSequence)
    {
        TArray<FBonePose> TargetPose;
        TargetSequence->GetAnimationPose(Mesh, TargetPose);
        Mesh->SetBoneLocalTransforms(TargetPose);
    }
}

void UAnimInstance::SetTargetSequence(UAnimSequence* InSequence, float InBlendTime)
{
    TargetSequence = InSequence;
    BlendTime = InBlendTime;
    ElapsedTime = 0.f;
}

void UAnimInstance::PlayAnimation(UAnimSequence* InSequence, bool bInLooping)
{
    InSequence->SetLooping(bInLooping);
}

void UAnimInstance::PlayAnimationByName(const FString& Name, bool bIsLooping)
{
    UAnimSequence* Sequence = FResourceManager::GetAnimationSequence(Name.ToWideString());
    if (!Sequence)
    {
        UE_LOG(LogLevel::Error, TEXT("Animation Sequence not found: %s"), *Name);
    }
    PlayAnimation(Sequence, bIsLooping);
}
