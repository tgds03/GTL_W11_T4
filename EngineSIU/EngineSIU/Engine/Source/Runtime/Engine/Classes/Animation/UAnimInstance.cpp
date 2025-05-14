#include "UAnimInstance.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"
#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "AnimationStateMachine.h"

#include <cmath>
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

void UAnimInstance::StartAnimSequence(UAnimSequence* InSequence)
{
    CurrentSequence = InSequence;
    CurrentGlobalTime = 0.0f;
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

    if (AnimStateMachine)
    {
        AnimStateMachine->ProcessState();
    }

    //바뀌면 애니메이션 체인지 -> 추가할지 바로바꿀지 결정
    if (CurrentState != AnimStateMachine->CurrentState)
    {
        if (!CurrentSequence)
        {   // 돌아가고 있는게 없으면 바로 변경
            StartAnimSequence(AnimSequenceMap[AnimStateMachine->CurrentState]);
        }
        else
        {   // 돌아가고 있는 애니메이션이 있으면 블렌드시키면서 변경
            ChangeAnimation(AnimSequenceMap[AnimStateMachine->CurrentState], 1.0f);
        }
    }
    
    CurrentState = AnimStateMachine->CurrentState;

    CurrentGlobalTime += DeltaSeconds;

    // Convert to local sequence time
    float LocalTime = CurrentSequence->GetLocalTime(CurrentGlobalTime);

    // Delegate pose calculation to the sequence
    TArray<FBonePose> NewLocalPoses;
    USkeletalMesh* Mesh = OwningComponent->GetSkeletalMesh();
    CurrentSequence->GetAnimationPose(LocalTime, Mesh, NewLocalPoses);

    //여기서 블렌딩로직 추가

    // Apply poses and update global transforms
    Mesh->SetBoneLocalTransforms(NewLocalPoses);
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSceonds)
{
}

void UAnimInstance::CheckAnimNotifyQueue()
{
    // 큐 초기화
    NotifyQueue.Reset();

    // 현재 재생 중인 애니메이션에서 노티파이 수집
    //if (CurrentSequence) {
    //    // 이전 프레임과 현재 프레임 사이에 있는 노티파이 찾기
    //    for (const FAnimNotifyEvent& Notify : CurrentSequence->Notifies) {
    //        if (Notify.TriggerTime > PreviousTime && Notify.TriggerTime <= CurrentTime) {
    //            NotifyQueue.AddAnimNotify(&Notify, CurrentSequence);
    //        }
    //    }
    //}
}

void UAnimInstance::ChangeAnimation(UAnimSequence* NewAnim, float InBlendingTime)
{
    BlendSequence = NewAnim;
    BlendTime = InBlendingTime;
}

void UAnimInstance::PlayAnimation(UAnimSequence* InSequence, bool bInLooping, bool bPlayDirect)
{
    InSequence->SetLooping(bInLooping);

    if (bPlayDirect)
    {
        StartAnimSequence(InSequence);
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

//void UAnimInstance::GetBoneTransforms(TArray<FBonePose>& OutTransforms)
//{
//    if (!OwningComponent)
//    {
//        return;
//    }
//
//    CurrentGlobalTime += DeltaSeconds;
//
//    // Convert to local sequence time
//    float LocalTime = CurrentSequence->GetLocalTime(CurrentGlobalTime);
//
//    // Delegate pose calculation to the sequence
//    TArray<FBonePose> NewLocalPoses;
//    USkeletalMesh* Mesh = OwningComponent->GetSkeletalMesh();
//    //장면전환해서 블렌드할 일이 생기면
//    bool bIsBlending = false;
//    UAnimDataModel* BlendModel = nullptr;
//    if (BlendSequence)
//    {
//        BlendModel = BlendSequence->GetDataModel();
//        bIsBlending = true;
//    }
//    
//    // 스켈레톤 정보 가져오기
//    const FSkeleton* Skeleton = SkeletalMesh->GetSkeleton();
//    int32 BoneCount = Skeleton->BoneCount;
//
//    CurrentSequence->GetAnimationPose(LocalTime, Mesh, NewLocalPoses);
//
//    // 본 트랜스폼 배열 초기화
//    OutTransforms.SetNum(BoneCount);
//
//    // 초기값은 바인드 포즈 트랜스폼
//    for (int32 i = 0; i < BoneCount; ++i)
//    {
//        OutTransforms[i] = SkeletonPose->LocalTransforms[i];
//    }
//
//    // 애니메이션 트랙 데이터 가져오기
//    const TArray<FBoneAnimationTrack>& Tracks = DataModel->GetBoneAnimationTracks();
//    float PlayLength = DataModel->GetPlayLength();
//    int32 NumFrames = DataModel->GetNumberOfFrames();
//    
//    // 로컬클록을 정규화
//    float NormalizedTime = FMath::Clamp(CurrentTime / PlayLength, 0.0f, 1.0f);
//    float FrameIndex = NormalizedTime * (NumFrames - 1);
//    int32 Frame1 = FMath::FloorToInt(FrameIndex);
//    int32 Frame2 = FMath::Min(Frame1 + 1, NumFrames - 1);
//    float AlphaTime = FrameIndex - Frame1;
//    
//    // 본 이름 -> 인덱스 매핑
//    TMap<FName, int32> BoneNameToIndexMap;
//    for (int32 i = 0; i < BoneCount; ++i)
//    {
//        BoneNameToIndexMap.Add(Skeleton->Bones[i].Name, i);
//    }
//
//    // 각 본 트랙에서 트랜스폼 계산
//    for (const FBoneAnimationTrack& Track : Tracks) 
//    {
//        const int32* BoneIndexPtr = BoneNameToIndexMap.Find(Track.Name);
//        if (!BoneIndexPtr)
//            continue;
//
//        int32 BoneIndex = *BoneIndexPtr;
//        const FRawAnimSequenceTrack& RawTrack = Track.InternalTrackData;
//        
//        // 위치 보간
//        FVector Position = FVector::ZeroVector;
//        if (RawTrack.PosKeys.Num() > 0)
//        {
//            int32 PosKeyLastIndex = RawTrack.PosKeys.Num() - 1;
//            int32 PosFrame1 = FMath::Min(Frame1, PosKeyLastIndex);
//            int32 PosFrame2 = FMath::Min(Frame2, PosKeyLastIndex);
//
//            //TODO: 조각적 선형근사로 변경
//            Position = FMath::Lerp(RawTrack.PosKeys[PosFrame1], RawTrack.PosKeys[PosFrame2], AlphaTime);
//        }
//
//        // 회전 보간
//        FQuat Rotation = FQuat::Identity;
//        if (RawTrack.RotKeys.Num() > 0)
//        {
//            int32 RotKeyLastIndex = RawTrack.RotKeys.Num() - 1;
//            int32 RotFrame1 = FMath::Min(Frame1, RotKeyLastIndex);
//            int32 RotFrame2 = FMath::Min(Frame2, RotKeyLastIndex);
//            
//            Rotation = FQuat::Slerp(RawTrack.RotKeys[RotFrame1], RawTrack.RotKeys[RotFrame2], AlphaTime);
//        }
//        Rotation.Normalize(); // ← 꼭 추가
//
//        // 스케일 보간
//        FVector Scale = FVector::OneVector;
//        if (RawTrack.ScaleKeys.Num() > 0)
//        {
//            int32 ScaleKeyLastIndex = RawTrack.ScaleKeys.Num() - 1;
//            int32 ScaleFrame1 = FMath::Min(Frame1, ScaleKeyLastIndex);
//            int32 ScaleFrame2 = FMath::Min(Frame2, ScaleKeyLastIndex);
//            
//            Scale = FMath::Lerp(RawTrack.ScaleKeys[ScaleFrame1], RawTrack.ScaleKeys[ScaleFrame2], AlphaTime);
//        }
//
//        if (bIsBlending)
//        {
//            Position *= 0.5f;
//            Rotation = Rotation * 0.5f;
//            Rotation.Normalize();
//            Scale *= 0.5f;            
//        }
//        
//        // 로컬 트랜스폼 설정
//        OutTransforms[BoneIndex] = FBonePose(Rotation, Position, Scale);
//    }
//
//    if (bIsBlending)
//    {
//        const TArray<FBoneAnimationTrack>& BlendTracks = BlendModel->GetBoneAnimationTracks();
//        float BlendPlayLength = BlendModel->GetPlayLength();
//        int32 BlendNumFrames = BlendModel->GetNumberOfFrames();
//
//        float BlendNormalizedTime = FMath::Clamp(BlendCurrentTime / BlendPlayLength, 0.0f, 1.0f);
//        float BlendFrameIndex = BlendNormalizedTime * (BlendNumFrames - 1);
//        int32 BlendFrame1 = FMath::FloorToInt(BlendFrameIndex);
//        int32 BlendFrame2 = FMath::Min(BlendFrame1 + 1, BlendNumFrames - 1);
//        float BlendAlphaTime = BlendFrameIndex - BlendFrame1;
//        
//        for (const FBoneAnimationTrack& Track : BlendTracks) 
//        {
//            const int32* BoneIndexPtr = BoneNameToIndexMap.Find(Track.Name);
//            if (!BoneIndexPtr)
//                continue;
//
//            int32 BoneIndex = *BoneIndexPtr;
//            const FRawAnimSequenceTrack& RawTrack = Track.InternalTrackData;
//            
//            // 위치 보간
//            FVector Position = FVector::ZeroVector;
//            if (RawTrack.PosKeys.Num() > 0)
//            {
//                int32 PosKeyLastIndex = RawTrack.PosKeys.Num() - 1;
//                int32 PosFrame1 = FMath::Min(BlendFrame1, PosKeyLastIndex);
//                int32 PosFrame2 = FMath::Min(BlendFrame2, PosKeyLastIndex);
//
//                //TODO: 조각적 선형근사로 변경
//                Position = FMath::Lerp(RawTrack.PosKeys[PosFrame1], RawTrack.PosKeys[PosFrame2], BlendAlphaTime);
//            }
//
//            // 회전 보간
//            FQuat Rotation = FQuat::Identity;
//            if (RawTrack.RotKeys.Num() > 0)
//            {
//                int32 RotKeyLastIndex = RawTrack.RotKeys.Num() - 1;
//                int32 RotFrame1 = FMath::Min(BlendFrame1, RotKeyLastIndex);
//                int32 RotFrame2 = FMath::Min(BlendFrame2, RotKeyLastIndex);
//                
//                Rotation = FQuat::Slerp(RawTrack.RotKeys[RotFrame1], RawTrack.RotKeys[RotFrame2], BlendAlphaTime);
//            }
//            Rotation.Normalize(); // ← 꼭 추가
//
//            // 스케일 보간
//            FVector Scale = FVector::OneVector;
//            if (RawTrack.ScaleKeys.Num() > 0)
//            {
//                int32 ScaleKeyLastIndex = RawTrack.ScaleKeys.Num() - 1;
//                int32 ScaleFrame1 = FMath::Min(BlendFrame1, ScaleKeyLastIndex);
//                int32 ScaleFrame2 = FMath::Min(BlendFrame2, ScaleKeyLastIndex);
//                
//                Scale = FMath::Lerp(RawTrack.ScaleKeys[ScaleFrame1], RawTrack.ScaleKeys[ScaleFrame2], BlendAlphaTime);
//            }
//
//            //절반가중치만큼만 적용
//            Position *= 0.5f;
//            Rotation = Rotation * 0.5f;
//            Rotation.Normalize();
//            Scale *= 0.5f;
//
//            FBonePose OriginPose = OutTransforms[BoneIndex];
//
//            OriginPose.Location += Position * 0.5f;
//            OriginPose.Rotation = OriginPose.Rotation * Rotation;
//            OriginPose.Scale += Scale;
//            
//            // 로컬 트랜스폼 설정
//            OutTransforms[BoneIndex] = OriginPose;
//        }
//
//        // 현재 애니메이션이 끝나거나 블렌드시간이 지나면 애니메이션 교체
//        if (BlendCurrentTime > BlendTime || CurrentTime > PlayLength)
//        {
//            CurrentTime = BlendCurrentTime;
//            CurrentSequence = BlendSequence;
//            BlendSequence = nullptr;
//            BlendTime = 0.f;
//            BlendCurrentTime = 0.f;
//        }
//    }
//}
