#include "AnimSequence.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"

UAnimSequence::UAnimSequence()
{
}

float UAnimSequence::GetLocalTime(float GlobalTime) const
{
    float LocalTime = GlobalTime * RateScale;
    float SequenceLength = GetUnScaledPlayLength();

    if (bLoopAnimation)
    {
        return FMath::Fmod(LocalTime, SequenceLength);
    }
    // 루프가 아닌 경우
    else
    {
        return FMath::Clamp(LocalTime, 0.0f, SequenceLength);
    }
}

void UAnimSequence::GetAnimationPose(float Time, USkeletalMesh* SkeletalMesh, TArray<FBonePose>& OutBoneTransforms) const
{
    if (!SkeletalMesh)
    {
        OutBoneTransforms.Empty();
        return;
    }

    const FSkeleton* Skeleton = SkeletalMesh->GetSkeleton();
    int32 BoneCount = Skeleton->BoneCount;

    FSkeletonPose* SkeletonPose = SkeletalMesh->GetSkeletonPose();
    OutBoneTransforms = SkeletonPose->LocalTransforms;

    const TArray<FBoneAnimationTrack>& Tracks = GetDataModel()->GetBoneAnimationTracks();
    float PlayLength = GetUnScaledPlayLength();
    int32 NumFrames = GetNumberOfFrames();

    float NormalizedTime = FMath::Clamp(Time / PlayLength, 0.0f, 1.0f);
    float FrameTime = NormalizedTime * (NumFrames - 1);
    int32 Frame1 = FMath::FloorToInt(FrameTime);
    int32 Frame2 = FMath::Min(Frame1 + 1, NumFrames - 1);
    float Alpha = FrameTime - Frame1;

    TMap<FName, int32> BoneIndexMap;
    for (int32 i = 0; i < BoneCount; ++i)
    {
        BoneIndexMap.Add(Skeleton->Bones[i].Name, i);
    }

    for (const FBoneAnimationTrack& Track : Tracks)
    {
        const int32* IndexPtr = BoneIndexMap.Find(Track.Name);
        if (!IndexPtr) continue;
        int32 BoneIndex = *IndexPtr;
        const FRawAnimSequenceTrack& Raw = Track.InternalTrackData;

        FVector Position = FVector::ZeroVector;
        FQuat Rotation = FQuat::Identity;
        FVector Scale = FVector::OneVector;

        InterpolateKeyframe(Raw, Frame1, Frame2, Alpha, Position, Rotation, Scale);

        Rotation.Normalize();
        OutBoneTransforms[BoneIndex] = FBonePose(Rotation, Position, Scale);
    }
}

void UAnimSequence::InterpolateKeyframe(const FRawAnimSequenceTrack& Track, int32 Frame1, int32 Frame2, float Alpha, FVector& OutPosition, FQuat& OutRotation, FVector& OutScale) const
{
    // 위치 보간
    if (Track.PosKeys.Num() > 0)
    {
        int32 PosFrame1 = FMath::Min(Frame1, Track.PosKeys.Num() - 1);
        int32 PosFrame2 = FMath::Min(Frame2, Track.PosKeys.Num() - 1);

        OutPosition = FMath::Lerp(Track.PosKeys[PosFrame1], Track.PosKeys[PosFrame2], Alpha);
    }
    // 회전 보간
    if (Track.RotKeys.Num() > 0)
    {
        int32 RotFrame1 = FMath::Min(Frame1, Track.RotKeys.Num() - 1);
        int32 RotFrame2 = FMath::Min(Frame2, Track.RotKeys.Num() - 1);
        if (RotFrame1 == RotFrame2 || Alpha < KINDA_SMALL_NUMBER)
        {
            OutRotation = Track.RotKeys[RotFrame1];
        }
        else
        {
            OutRotation = FQuat::Slerp(Track.RotKeys[RotFrame1], Track.RotKeys[RotFrame2], Alpha);
        }
    }
    // 스케일 보간
    if (Track.ScaleKeys.Num() > 0)
    {
        int32 ScaleFrame1 = FMath::Min(Frame1, Track.ScaleKeys.Num() - 1);
        int32 ScaleFrame2 = FMath::Min(Frame2, Track.ScaleKeys.Num() - 1);

        OutScale = FMath::Lerp(Track.ScaleKeys[ScaleFrame1], Track.ScaleKeys[ScaleFrame2], Alpha);
    }
}


