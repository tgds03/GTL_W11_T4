#pragma once

#include "UAnimDataModel.h"
#include "Launch/SkeletalDefine.h";

class USkeletalMeshComponent;
class UAnimSequence;

enum class EAnimState
{
    Idle,
    Twerk,
};

class UAnimInstance : public UObject
{
protected:
    // 소유 컴포넌트
    USkeletalMeshComponent* OwningComponent;

    TMap<EAnimState, UAnimSequence*> AnimSequenceMap;

    UAnimSequence* CurrentSequence = nullptr;
    UAnimSequence* BlendSequence = nullptr;

    // 재생 상태
    bool bIsPlaying = true;
    float CurrentGlobalTime = 0;


public:
    UAnimInstance();
    virtual ~UAnimInstance() = default;

    // 컴포넌트와 연결
    void Initialize(USkeletalMeshComponent* InComponent);
    void SetAnimSequence(UAnimSequence* InSequence);

    // 매 프레임 업데이트
    void Update(float DeltaTime);
    virtual void NativeUpdateAnimation(float DeltaSeconds);

    //void TriggerAnimNotifies(float DeltaSceonds);

#pragma region Properties
    USkeletalMeshComponent* GetOwningComponent() const { return OwningComponent; }
    void SetOwningComponent(USkeletalMeshComponent* InComponent) { OwningComponent = InComponent; }

    // 현재 애니메이션 접근자
    UAnimSequence* GetCurrentAnimSequence() const { return CurrentSequence; }
    void SetAnimaSequence(UAnimSequence* AnimSeq) { CurrentSequence = AnimSeq; }

    bool IsPlaying() const { return bIsPlaying; }
#pragma endregion

    //void GetBoneTransforms(TArray<FBonePose>& OutTransforms);

protected:
    // 애니메이션 노티파이 처리
    //void ProcessNotifies(float PreviousTime, float CurrentTime);

    // 애니메이션 상태 업데이트
    //void UpdateAnimationState(float DeltaTime);

    // 포즈 계산
    //void CalculatePose();
};
