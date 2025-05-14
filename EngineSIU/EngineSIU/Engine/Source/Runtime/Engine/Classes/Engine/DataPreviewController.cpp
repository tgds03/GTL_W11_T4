#include "DataPreviewController.h"
#include "InteractiveToolsFramework/BoneGizmos/ABoneGizmo.h"
#include "Components/SkeletalMesh/SkeletalmeshComponent.h"
#include "Animation/UAnimInstance.h"
#include "Engine.h"
#include "World/World.h"
#include "Engine/Source/Editor/UnrealEd/EditorViewportClient.h"

void UDataPreviewController::Initialize(USkeletalMesh* InMesh)
{
    OriginalMesh = InMesh;
    EdittingMesh = OriginalMesh->DuplicateSkeletalMesh();

    AttachedViewport->SetViewMode(EViewModeIndex::VMI_Unlit);

    SetType(EPreviewType::SkeletalMesh);

    SetBoneGizmo(OriginalMesh);
}

void UDataPreviewController::Initialize(UAnimInstance* InAnim)
{
    USkeletalMesh* InMesh = InAnim->GetOwningComponent()->GetSkeletalMesh();

    if (!InMesh)
    {
        // TODO: Default Mesh로 초기화
        UE_LOG(LogLevel::Error, TEXT("InMesh is null"));
        return;
    }

    OriginalMesh = InMesh;
    EdittingMesh = OriginalMesh->DuplicateSkeletalMesh();

    OriginalAnim = InAnim;
    EditingAnim = OriginalAnim; // TODO: 복제 함수 추가 필요
    //EditingAnim = OriginalAnim->DuplicateAnimInstance();

    SetType(EPreviewType::Animation);

    SetBoneGizmo(OriginalMesh);
}

void UDataPreviewController::Release()
{
    AttachedViewport = nullptr;
    OriginalMesh = nullptr;
    EdittingMesh = nullptr;
    OriginalAnim = nullptr;
    EditingAnim = nullptr;
    SelectedGizmo = nullptr; 
    BoneGizmos.Empty();
}

void UDataPreviewController::SetBoneGizmo(USkeletalMesh* InMesh)
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

int UDataPreviewController::GetBoneIndex(ABoneGizmo* InBoneGizmo)
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

int UDataPreviewController::GetSelectedBoneIndex()
{
    return GetBoneIndex(SelectedGizmo);
}

void UDataPreviewController::SetSelectedBoneIndex(int index)
{
    if (index >= BoneGizmos.Num()) 
    {
        SelectedGizmo = BoneGizmos[index];
        return;
    }

    SelectedGizmo = nullptr;
}

