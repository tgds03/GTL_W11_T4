#include "ABoneGizmo.h"
#include "Launch/SkeletalDefine.h"
#include "BoneGizmos/UGizmoJointComponent.h"
#include "BoneGizmos/UGizmoFrameComponent.h"

ABoneGizmo::ABoneGizmo()
{
    FResourceManager::CreateStaticMesh("Assets/Gizmo/BoneFrame.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/BoneJoint.obj");

    SetRootComponent(AddComponent<USceneComponent>());

    Joint = AddComponent<UGizmoJointComponent>();
    Joint->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/BoneJoint.obj"));
    Joint->SetupAttachment(RootComponent);
    Joint->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    
    Frame = AddComponent<UGizmoFrameComponent>();
    Frame->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/BoneFrame.obj"));
    Frame->SetupAttachment(RootComponent);
    Frame->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
}

void ABoneGizmo::Tick(float DeltaTime)
{
    UpdatePose();
}

void ABoneGizmo::Initialize(FEditorViewportClient* InViewport)
{
    AttachedViewport = InViewport;
}

void ABoneGizmo::SetPose(FBone* InBone, TArray<FBone>& Bones)
{
    TargetBone = InBone;
    TargetBones = &Bones;
    UpdatePose();
}

void ABoneGizmo::UpdatePose()
{
    if (TargetBone)
    {
        SetActorLocation(TargetBone->GlobalTransform.GetTranslationVector());

        TArray<int32> ChildIndices = TargetBone->Children;
        if (ChildIndices.Num() > 0)
        {
            // 첫 번째 자식만 예시로
            FBone& ChildBone = (*TargetBones)[ChildIndices[0]];

            // 월드 공간상의 위치
            FVector ParentPos = TargetBone->GlobalTransform.GetTranslationVector();
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
            SetActorRotation(RotQuat.Rotator());

            // 거리 계산
            float Distance = FVector::Distance(ParentPos, ChildPos);

            // Z축 스케일에 반영 (원본 메시가 Z=1 길이라면)
            SetActorScale(FVector(Distance, Distance, Distance));
        }
    }
}


