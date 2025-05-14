#pragma once
#include "UObject/Object.h"
#include "AnimTypes.h"

struct FBonePose;
class APawn;

enum EAnimState
{
    AS_Idle,
    AS_Work,
    AS_Run,
    AS_Fly,
    AS_Dance,
    AS_Die,
};

class UAnimationStateMachine : public UObject
{
public:

    virtual void Initialize(APawn* InOwner);
    
    void ProcessState();
    void StartAnimSequence(UAnimSequence* InSequence, float InBlendingTime);
    void UpdateSequence(float DeltaTime, USkeletalMesh* InSkeletalMesh);

    void SetAnimationTime(float Time);

    APawn* Owner;

    EAnimState CurrentState;
    EAnimState PreState;
    
    TMap<EAnimState, UAnimSequence*> AnimSequenceMap;
    float BlendTime = 0.f;

    TArray<FBonePose> CurrentPose;
    
    UAnimSequence* CurrentSequence = nullptr;
    UAnimSequence* BlendSequence = nullptr;
    
    void AddAnimSequence(EAnimState InAnimState, UAnimSequence* InAnimSequence){ AnimSequenceMap.Add(InAnimState, InAnimSequence); }
    UAnimSequence* GetAnimSequence(EAnimState InAnimState){ return AnimSequenceMap[InAnimState]; }
    
    // 현재 애니메이션 접근자
    UAnimSequence* GetCurrentAnimSequence() const { return CurrentSequence; }
    void SetAnimaSequence(UAnimSequence* AnimSeq) { CurrentSequence = AnimSeq; }
    
    TArray<FBonePose> GetCurrentPose() { return CurrentPose; }
    
};
