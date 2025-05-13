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

    void UpdateAnimation(float deltatime);

    void PlayAnimation(UAnimationAsset* NewAnimToPlay, bool bLooping = false);
    //void SetAnimationMode(EAnimationMode AnimationMode);
    void SetAnimation(UAnimationAsset* InAnimAsset);
    void Play(bool bLooping = false);
    void HandleAnimNotify(const FAnimNotifyEvent* Notify);
    
    virtual void TickComponent(float DeltaTime) override;
    virtual void TickPose(float DeltaTime) override;
    void TickAnimation(float DeltaTime);

    void InitializeAnimInstance(APawn* InOwner);

    // OBJ 파일을 통한 Sample Skeletal Mesh 구현
    void GenerateSampleData();

    // 각도 변화 주기
    void TestSkeletalMesh();

    void TestFBXSkeletalMesh(); 

    void PerformCPUSkinning();
};
