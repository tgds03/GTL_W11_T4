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
        BoneGizmo->SetPose(&Bone, InSkeleton->Bones);

        BoneGizmos.Add(BoneGizmo);
    }
}
