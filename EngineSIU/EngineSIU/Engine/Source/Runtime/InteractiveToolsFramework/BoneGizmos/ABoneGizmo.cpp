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
    //
}

void ABoneGizmo::Initialize(FEditorViewportClient* InViewport)
{
    AttachedViewport = InViewport;
}

void ABoneGizmo::SetPose(FBone* InBone)
{
    TargetBone = InBone;
    if (TargetBone)
    {
        SetActorLocation(TargetBone->GlobalTransform.GetTranslationVector()); // TODO : GlobalLocation으로 변경
        SetActorRotation(TargetBone->LocalTransform.Rotation.Rotator());
    }
}
