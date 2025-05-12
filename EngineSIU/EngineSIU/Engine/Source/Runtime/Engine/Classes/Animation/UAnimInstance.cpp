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
}

void UAnimInstance::SetAnimSequence(UAnimSequence* InSequence)
{
    CurrentSequence = InSequence;
    CurrentGlobalTime = 0.0f;
}

void UAnimInstance::Update(float DeltaTime)
{
    if (!bIsPlaying || !CurrentSequence)
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

    CurrentGlobalTime += DeltaSeconds;

    // Convert to local sequence time
    float LocalTime = CurrentSequence->GetLocalTime(CurrentGlobalTime);

    // Delegate pose calculation to the sequence
    TArray<FBonePose> NewLocalPoses;
    USkeletalMesh* Mesh = OwningComponent->GetSkeletalMesh();
    CurrentSequence->GetAnimationPose(LocalTime, Mesh, NewLocalPoses);

    // Apply poses and update global transforms

    Mesh->SetBoneLocalTransforms(NewLocalPoses);
}

//void UAnimInstance::GetBoneTransforms(TArray<FBonePose>& OutTransforms)
//{
//    if (!OwningComponent || !CurrentSequence)
//    {
//        OutTransforms.Empty();
//        return;
//    }
//
//    USkeletalMesh* SkeletalMesh = OwningComponent->GetSkeletalMesh();
//    if (!SkeletalMesh)
//    {
//        OutTransforms.Empty();
//        return;
//    }
//
//    // 애니메이션 데이터 모델 가져오기
//    UAnimDataModel* DataModel = CurrentSequence->GetDataModel();
//    if (!DataModel)
//    {
//        OutTransforms.Empty();
//        return;
//    }
//
//    // 스켈레톤 정보 가져오기
//    const FSkeleton* Skeleton = SkeletalMesh->GetSkeleton();
//    int32 BoneCount = Skeleton->BoneCount;
//
//    FSkeletonPose* SkeletonPose = SkeletalMesh->GetSkeletonPose();
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
//    // 현재 재생 시간으로 프레임 인덱스 계산
//    float NormalizedTime = FMath::Clamp(CurrentTime / PlayLength, 0.0f, 1.0f);
//    float FrameTime = NormalizedTime * (NumFrames - 1);
//    int32 Frame1 = FMath::FloorToInt(FrameTime);
//    int32 Frame2 = FMath::Min(Frame1 + 1, NumFrames - 1);
//    float Alpha = FrameTime - Frame1;
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
//            int32 PosFrame1 = FMath::Min(Frame1, RawTrack.PosKeys.Num() - 1);
//            int32 PosFrame2 = FMath::Min(Frame2, RawTrack.PosKeys.Num() - 1);
//
//            if (PosFrame1 == PosFrame2 || Alpha < KINDA_SMALL_NUMBER)
//            {
//                Position = RawTrack.PosKeys[PosFrame1];
//            }
//            else
//            {
//                Position = FMath::Lerp(RawTrack.PosKeys[PosFrame1], RawTrack.PosKeys[PosFrame2], Alpha);
//            }
//        }
//
//        // 회전 보간
//        FQuat Rotation = FQuat::Identity;
//        if (RawTrack.RotKeys.Num() > 0)
//        {
//            int32 RotFrame1 = FMath::Min(Frame1, RawTrack.RotKeys.Num() - 1);
//            int32 RotFrame2 = FMath::Min(Frame2, RawTrack.RotKeys.Num() - 1);
//
//            if (RotFrame1 == RotFrame2 || Alpha < KINDA_SMALL_NUMBER)
//            {
//                Rotation = RawTrack.RotKeys[RotFrame1];
//            }
//            else
//            {
//                Rotation = FQuat::Slerp(RawTrack.RotKeys[RotFrame1], RawTrack.RotKeys[RotFrame2], Alpha);
//            }
//        }
//        Rotation.Normalize(); // ← 꼭 추가
//
//        // 스케일 보간
//        FVector Scale = FVector::OneVector;
//        if (RawTrack.ScaleKeys.Num() > 0)
//        {
//            int32 ScaleFrame1 = FMath::Min(Frame1, RawTrack.ScaleKeys.Num() - 1);
//            int32 ScaleFrame2 = FMath::Min(Frame2, RawTrack.ScaleKeys.Num() - 1);
//
//            if (ScaleFrame1 == ScaleFrame2 || Alpha < KINDA_SMALL_NUMBER)
//            {
//                Scale = RawTrack.ScaleKeys[ScaleFrame1];
//            }
//            else
//            {
//                Scale = FMath::Lerp(RawTrack.ScaleKeys[ScaleFrame1], RawTrack.ScaleKeys[ScaleFrame2], Alpha);
//            }
//        }
//
//        // 로컬 트랜스폼 설정
//        OutTransforms[BoneIndex] = FBonePose(Rotation, Position, Scale);
//    }
//
//}
