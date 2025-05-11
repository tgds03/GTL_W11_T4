#include "SkeletalMeshEditorController.h"
#include "InteractiveToolsFramework/BoneGizmos/ABoneGizmo.h"
#include "Engine.h"
#include "World/World.h"

void SkeletalMeshEditorController::Initialize(USkeletalMesh* InMesh, FEditorViewportClient* InViewport)
{
    OriginalMesh = InMesh;
    EditingMesh = OriginalMesh->DuplicateSkeletalMesh();
    AttachedViewport = InViewport;

    SetBoneGizmo(OriginalMesh);
}

void SkeletalMeshEditorController::Release()
{
}

void SkeletalMeshEditorController::SetBoneGizmo(USkeletalMesh* InMesh)
{
    if (InMesh == nullptr)
        return;

    for (ABoneGizmo* OldGizmo : BoneGizmos)
    {
        if (OldGizmo)
        {
            OldGizmo->Destroy();
        }
    }
    BoneGizmos.Empty();

    for (int i = 0; i < InMesh->GetSkeleton()->BoneCount; ++i)
    {
        FBone& Bone = InMesh->GetSkeleton()->Bones[i];

        ABoneGizmo* BoneGizmo = GEngine->ActiveWorld->SpawnActor<ABoneGizmo>();
        BoneGizmo->Initialize(AttachedViewport);
        BoneGizmo->SetActorLabel(Bone.Name.ToString());
        BoneGizmo->SetPose(InMesh, i);

        BoneGizmos.Add(BoneGizmo);
    }
}

int SkeletalMeshEditorController::GetBoneIndex(ABoneGizmo* InBoneGizmo)
{
    for (int i = 0; i < BoneGizmos.Num(); i++) 
    {
        if (BoneGizmos[i] == InBoneGizmo) 
        {
            return i;
        }
    }
    return -1;
}

int SkeletalMeshEditorController::GetSelectedBoneIndex()
{
    return GetBoneIndex(SelectedGizmo);
}

void SkeletalMeshEditorController::SetSelectedBoneIndex(int index)
{
    if (index >= BoneGizmos.Num()) 
    {
        SelectedGizmo = BoneGizmos[index];
        return;
    }

    SelectedGizmo = nullptr;
}

