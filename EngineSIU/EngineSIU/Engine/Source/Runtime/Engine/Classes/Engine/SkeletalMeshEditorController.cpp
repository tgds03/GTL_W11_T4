#include "SkeletalMeshEditorController.h"
#include "InteractiveToolsFramework/BoneGizmos/ABoneGizmo.h"
#include "Engine.h"
#include "World/World.h"

void SkeletalMeshEditorController::Initialize(USkeletalMesh* InMesh, FEditorViewportClient* InViewport)
{
    OriginalMesh = InMesh;
    EditingMesh = OriginalMesh->DuplicateSkeletalMesh();
    AttachedViewport = InViewport;

    SetBoneGizmo(OriginalMesh->GetSkeleton());
}

void SkeletalMeshEditorController::Release()
{
}

void SkeletalMeshEditorController::SetBoneGizmo(FSkeleton* InSkeleton)
{
    if (InSkeleton == nullptr)
        return;

    for (ABoneGizmo* OldGizmo : BoneGizmos)
    {
        if (OldGizmo)
        {
            OldGizmo->Destroy();
        }
    }
    BoneGizmos.Empty();

    for (int i = 0; i < InSkeleton->BoneCount; ++i)
    {
        FBone& Bone = InSkeleton->Bones[i];

        ABoneGizmo* BoneGizmo = GEngine->ActiveWorld->SpawnActor<ABoneGizmo>();
        BoneGizmo->Initialize(AttachedViewport);
        BoneGizmo->SetActorLabel(Bone.Name.ToString());
        BoneGizmo->SetPose(&Bone);

        TArray<int32> ChildIndices = Bone.Children;
        if (ChildIndices.Num() > 0)
        {
            // 첫 번째 자식만 예시로
            FBone& ChildBone = InSkeleton->Bones[ChildIndices[0]];

            // 월드 공간상의 위치
            FVector ParentPos = Bone.GlobalTransform.GetTranslationVector();
            FVector ChildPos = ChildBone.GlobalTransform.GetTranslationVector();

            // 2) 방향 벡터 계산
            FVector Dir = (ChildPos - ParentPos).GetSafeNormal();

            // 3) Z-Up 축과 Dir 사이 회전 쿼터니언 계산
            const FVector Up(0, 0, 1);
            float Dot = FMath::Clamp(FVector::DotProduct(Up, Dir), -1.0f, 1.0f);
            FVector Axis = Up ^ Dir;
            if (Axis.IsNearlyZero())
            {
                // Up과 Dir이 거의 같거나 반대일 때 임의 축
                Axis = FVector(1, 0, 0);
            }
            Axis.Normalize();
            float Angle = FMath::Acos(Dot);
            FQuat RotQuat(Axis, Angle);

            // 4) 회전 및 스케일 적용
            BoneGizmo->SetActorRotation(RotQuat.Rotator());

            // 거리 계산
            float Distance = FVector::Distance(ParentPos, ChildPos);

            // Z축 스케일에 반영 (원본 메시가 Z=1 길이라면)
            BoneGizmo->SetActorScale(FVector(Distance, Distance, Distance));
        }

        BoneGizmos.Add(BoneGizmo);
    }
}
