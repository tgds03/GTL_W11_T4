#include "UAnimInstance.h"
#include <Engine/SkeletalMeshEditorController.h>
#include "Components/SkeletalMesh/SkeletalMesh.h"
#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include <cmath>
UAnimInstance::UAnimInstance()
{
}

void UAnimInstance::Initialize(USkeletalMeshComponent* InComponent)
{
    OwningComponent = InComponent;
    CurrentTime = 0.0f;
    PlayRate = 1.0f;
}

void UAnimInstance::StartAnimSequence(UAnimSequence* InSequence)
{
    CurrentSequence = InSequence;
    CurrentTime = 0.0f; // 애니메이션 시간 초기화
    OnAnimationNotifyDelegate["OnAnimationStart"].Broadcast();
}

void UAnimInstance::Update(float DeltaTime)
{
    if (!bIsPlaying)
        return;

    if (!CurrentSequence)
    {
        if (WaitSequences.IsEmpty())
        {
            return;
        }

        WaitSequences.Dequeue(CurrentSequence);
    }
    
    // 1. 애니메이션 시간 업데이트
    CurrentTime += DeltaTime * PlayRate;

    // 2. 애니메이션 데이터 모델 가져오기
    UAnimDataModel* DataModel = CurrentSequence->GetDataModel();
    if (!DataModel)
        return;

    // 3. 애니메이션 끝에 도달했는지 체크 (루핑 처리)
    float Duration = DataModel->GetPlayLength();
    if (Duration > 0.0f && CurrentTime >= Duration)
    {
        if (CurrentSequence->IsLooping())
        {
            //루프시작
            OnAnimationNotifyDelegate["OnAnimationLoop"].Broadcast();
            
            // 루프 처리
            CurrentTime = fmod(CurrentTime, Duration);
        }
        else
        {
            // 재생 종료
            OnAnimationNotifyDelegate["OnAnimationEnd"].Broadcast();
            StopAnimation();
        }
    }

    // 4. 필요한 경우 애니메이션 이벤트(노티파이) 처리
    // (이 부분은 필요에 따라 구현)
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

void UAnimInstance::StopAnimation()
{
    CurrentSequence = nullptr;
    CurrentTime = 0.0f;
    bIsPlaying = false;
}

bool UAnimInstance::IsLooping() const
{
    if (CurrentSequence)
    {
        return CurrentSequence->IsLooping();
    }

    return false;
}

void UAnimInstance::GetBoneTransforms(TArray<FBonePose>& OutTransforms)
{
    if (!OwningComponent || !CurrentSequence)
    {
        OutTransforms.Empty();
        return;
    }

    USkeletalMesh* SkeletalMesh = OwningComponent->GetSkeletalMesh();
    if (!SkeletalMesh)
    {
        OutTransforms.Empty();
        return;
    }

    // 애니메이션 데이터 모델 가져오기
    UAnimDataModel* DataModel = CurrentSequence->GetDataModel();
    if (!DataModel)
    {
        OutTransforms.Empty();
        return;
    }

    // 스켈레톤 정보 가져오기
    const FSkeleton* Skeleton = SkeletalMesh->GetSkeleton();
    int32 BoneCount = Skeleton->BoneCount;

    FSkeletonPose* SkeletonPose = SkeletalMesh->GetSkeletonPose();

    // 본 트랜스폼 배열 초기화
    OutTransforms.SetNum(BoneCount);

    // 초기값은 바인드 포즈 트랜스폼
    for (int32 i = 0; i < BoneCount; ++i)
    {
        OutTransforms[i] = SkeletonPose->LocalTransforms[i];
    }

    // 애니메이션 트랙 데이터 가져오기
    const TArray<FBoneAnimationTrack>& Tracks = DataModel->GetBoneAnimationTracks();
    float PlayLength = DataModel->GetPlayLength();
    int32 NumFrames = DataModel->GetNumberOfFrames();
    
    // 로컬클록을 정규화
    float NormalizedTime = FMath::Clamp(CurrentTime / PlayLength, 0.0f, 1.0f);
    float FrameIndex = NormalizedTime * (NumFrames - 1);
    int32 Frame1 = FMath::FloorToInt(FrameIndex);
    //TODO: 다음 애니메이션 이어붙이기 해서 Min빼기 
    int32 Frame2 = FMath::Min(Frame1 + 1, NumFrames - 1);
    float AlphaTime = FrameIndex - Frame1;

    // 본 이름 -> 인덱스 매핑
    TMap<FName, int32> BoneNameToIndexMap;
    for (int32 i = 0; i < BoneCount; ++i)
    {
        BoneNameToIndexMap.Add(Skeleton->Bones[i].Name, i);
    }

    // 각 본 트랙에서 트랜스폼 계산
    for (const FBoneAnimationTrack& Track : Tracks) 
    {
        const int32* BoneIndexPtr = BoneNameToIndexMap.Find(Track.Name);
        if (!BoneIndexPtr)
            continue;

        int32 BoneIndex = *BoneIndexPtr;
        const FRawAnimSequenceTrack& RawTrack = Track.InternalTrackData;
        
        // 위치 보간
        FVector Position = FVector::ZeroVector;
        if (RawTrack.PosKeys.Num() > 0)
        {
            int32 PosKeyLastIndex = RawTrack.PosKeys.Num() - 1;
            int32 PosFrame1 = FMath::Min(Frame1, PosKeyLastIndex);
            int32 PosFrame2 = FMath::Min(Frame2, PosKeyLastIndex);

            //TODO: 조각적 선형근사로 변경
            Position = FMath::Lerp(RawTrack.PosKeys[PosFrame1], RawTrack.PosKeys[PosFrame2], AlphaTime);
        }

        // 회전 보간
        FQuat Rotation = FQuat::Identity;
        if (RawTrack.RotKeys.Num() > 0)
        {
            int32 RotKeyLastIndex = RawTrack.RotKeys.Num() - 1;
            int32 RotFrame1 = FMath::Min(Frame1, RotKeyLastIndex);
            int32 RotFrame2 = FMath::Min(Frame2, RotKeyLastIndex);
            
            Rotation = FQuat::Slerp(RawTrack.RotKeys[RotFrame1], RawTrack.RotKeys[RotFrame2], AlphaTime);
        }
        Rotation.Normalize(); // ← 꼭 추가

        // 스케일 보간
        FVector Scale = FVector::OneVector;
        if (RawTrack.ScaleKeys.Num() > 0)
        {
            int32 ScaleKeyLastIndex = RawTrack.ScaleKeys.Num() - 1;
            int32 ScaleFrame1 = FMath::Min(Frame1, ScaleKeyLastIndex);
            int32 ScaleFrame2 = FMath::Min(Frame2, ScaleKeyLastIndex);
            
            Scale = FMath::Lerp(RawTrack.ScaleKeys[ScaleFrame1], RawTrack.ScaleKeys[ScaleFrame2], AlphaTime);
        }

        // 로컬 트랜스폼 설정
        OutTransforms[BoneIndex] = FBonePose(Rotation, Position, Scale);
    }

}
