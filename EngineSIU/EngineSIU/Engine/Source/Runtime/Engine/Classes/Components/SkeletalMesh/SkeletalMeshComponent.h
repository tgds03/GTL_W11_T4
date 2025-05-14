#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Launch/SkeletalDefine.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/SkeletalMesh/SkinnedMeshComponent.h"
#include "Animation/UAnimInstance.h"

class UAnimationAsset;
class FAnimNotifyEvent;

class USkeletalMeshComponent : public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)

    std::shared_ptr<UAnimInstance> AnimInstance = nullptr;

public:
    USkeletalMeshComponent();
    ~USkeletalMeshComponent() = default;

    void InitializeAnimInstance(APawn* InOwner);

    void PlayAnimation(UAnimationAsset* NewAnimToPlay, bool bLooping = false);
    //void SetAnimationMode(EAnimationMode AnimationMode);
    std::shared_ptr<UAnimInstance> GetAnimInstance() const { return AnimInstance; }
    void SetAnimInstance(std::shared_ptr<UAnimInstance> InAnimInstance) { AnimInstance = InAnimInstance; }
    void SetAnimation(UAnimationAsset* InAnimAsset);
    void Play(bool bLooping = false);
    void HandleAnimNotify(const FAnimNotifyEvent* Notify);
    
    virtual void TickComponent(float DeltaTime) override;
    virtual void TickPose(float DeltaTime) override;
    void TickAnimation(float DeltaTime);

    // 각도 변화 주기
    void LoadAndSetFBX(FString FileName); 
    void LoadAndSetAnimation(FString FileName);

    void TestAnimationStateMachine();
    void SwitchState();

    void PerformCPUSkinning();

    APawn* OwnerPawn = nullptr;
};
