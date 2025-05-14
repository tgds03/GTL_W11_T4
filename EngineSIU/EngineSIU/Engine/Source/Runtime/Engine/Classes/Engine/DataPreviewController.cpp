#include "DataPreviewController.h"
#include "InteractiveToolsFramework/BoneGizmos/ABoneGizmo.h"
#include "Components/SkeletalMesh/SkeletalmeshComponent.h"
#include "Animation/UAnimInstance.h"
#include "Engine.h"
#include "World/World.h"
#include "Engine/Source/Editor/UnrealEd/EditorViewportClient.h"
#include "Actors/Character/Pawn.h"

void UDataPreviewController::Initialize(USkeletalMesh* InMesh)
{
    OriginalMesh = InMesh;
    EdittingMesh = OriginalMesh->DuplicateSkeletalMesh();

    AttachedViewport->SetViewMode(EViewModeIndex::VMI_Unlit);

    SetType(EPreviewType::SkeletalMesh);
    isVisibleBone = true;
    SetBoneGizmo(OriginalMesh);
}

void UDataPreviewController::Initialize(UAnimInstance* InAnim)
{
    if (!InAnim)
    {
        return;
    }

    USkeletalMesh* InMesh = InAnim->GetOwningComponent()->GetSkeletalMesh();
    if (!InMesh)
    {
        // TODO: Default Mesh로 초기화
        UE_LOG(LogLevel::Error, TEXT("InMesh is null"));
        return;
    }

    OriginalMesh = InMesh;
    EdittingMesh = OriginalMesh->DuplicateSkeletalMesh();

    //EditingAnim = OriginalAnim->DuplicateAnimInstance();

    APawn* PreviewActor = PreviewWorld->SpawnActor<APawn>();
    PreviewActor->SetActorLabel(FString(TEXT("Animation Preview Actor")));

    USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(PreviewActor->GetRootComponent());
    SkelComp->SetRelativeRotation(FRotator(0, 0, -90));
    SkelComp->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
    UAnimSequence* CopyAnimSequence = Cast<UAnimSequence>(InAnim->GetCurrentSequence()->Duplicate(PreviewWorld));
    SkelComp->GetAnimInstance()->SetTargetSequence(CopyAnimSequence, 0.0f);
    SkelComp->SetSkeletalMesh(OriginalMesh);

    OriginalAnim = InAnim;
    EdittingAnim = SkelComp->GetAnimInstance(); // TODO: 복제 함수 추가 필요

    SetType(EPreviewType::Animation);
    isVisibleBone = false;
    SetBoneGizmo(OriginalMesh);
}

void UDataPreviewController::Release()
{
    AttachedViewport = nullptr;
    OriginalMesh = nullptr;
    EdittingMesh = nullptr;
    OriginalAnim = nullptr;
    EdittingAnim = nullptr;
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

