#include "SkeletonMeshEditorPanel.h"
#include "UnrealEd/ImGuiWidget.h"
#include "Engine/EditorEngine.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"
#include "Math/Quat.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Math/JungleMath.h"
#include "Engine/SkeletalMeshEditorController.h"

void SkeletonMeshEditorPanel::Render()
{
    // 에디터 엔진에서 현재 편집 중인 SkeletalMesh를 가져옴
    USkeletalMesh* SkelMesh = Cast<UEditorEngine>(GEngine)
        ->GetSkeletalMeshEditorController()
        ->OriginalMesh;
    if (!SkelMesh)
    {
        ImGui::Text("No SkeletalMesh selected.");
        return;
    }

    auto& Bones = SkelMesh->GetSkeleton()->Bones;

    ImGui::Begin("Skeletal Mesh Editor", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // 루트 본 찾기
    int32 RootIdx = INDEX_NONE;
    for (int32 i = 0; i < Bones.Num(); ++i)
    {
        if (Bones[i].ParentIndex == INDEX_NONE)
        {
            RootIdx = i;
            break;
        }
    }

    if (ImGui::Button("Exit Edit Mode"))
    {
        if (UEditorEngine* Engine = Cast<UEditorEngine>(GEngine))
        {
            Engine->EndSkeletalMeshEditMode();
        }
    }

    if (RootIdx != INDEX_NONE)
    {
        ImGui::Text("Bone Hierarchy:");
        ImGui::Separator();
        RenderBoneTree(RootIdx);
    }

    ImGui::Separator();


    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    SelectedBoneIndex = Engine->GetSkeletalMeshEditorController()->GetSelectedBoneIndex();

    // 선택된 본의 FBonePose 편집
    if (SelectedBoneIndex != INDEX_NONE)
    {
        FBone& Bone = Bones[SelectedBoneIndex];
        ImGui::Text("Edit Bone: %s", *Bone.Name.ToString());
        ImGui::Spacing();

        // 현재 로컬 포즈 복사
        FBonePose EditingPose = SkelMesh->GetLocalTransforms()[SelectedBoneIndex];

        // 위치
        float loc[3] = { EditingPose.Location.X, EditingPose.Location.Y, EditingPose.Location.Z };
        if (ImGui::DragFloat3("Location", loc, 0.01f))
        {
            EditingPose.Location = FVector(loc[0], loc[1], loc[2]);
        }

        // 회전 (Euler → Quaternion)
        FRotator rot = EditingPose.Rotation.Rotator();
        float originEul[3] = { rot.Pitch, rot.Yaw, rot.Roll };
        float nextEul[3] = { rot.Pitch, rot.Yaw, rot.Roll };
        if (ImGui::DragFloat3("Rotation (P,Y,R)", nextEul, 0.5f))
        {
            EditingPose.Rotation = EditingPose.Rotation * JungleMath::EulerToQuaternion(FVector(nextEul[0] - originEul[0], nextEul[1] - originEul[1], nextEul[2] - originEul[2]));
        }

        // 스케일
        float scl[3] = { EditingPose.Scale.X, EditingPose.Scale.Y, EditingPose.Scale.Z };
        if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.01f, 100.f))
        {
            EditingPose.Scale = FVector(scl[0], scl[1], scl[2]);
        }

        SkelMesh->GetLocalTransforms()[SelectedBoneIndex] = EditingPose;
        SkelMesh->UpdateGlobalTransforms();
    }
    else
    {
        ImGui::Text("Select a bone above to edit its pose.");
    }

    ImGui::End();
}

void SkeletonMeshEditorPanel::RenderBoneTree(int32 BoneIndex)
{
    USkeletalMesh* SkelMesh = Cast<UEditorEngine>(GEngine)
        ->GetSkeletalMeshEditorController()
        ->OriginalMesh;
    const auto& Bones = SkelMesh->GetSkeleton()->Bones;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
        | ImGuiTreeNodeFlags_SpanAvailWidth
        | ((SelectedBoneIndex == BoneIndex) ? ImGuiTreeNodeFlags_Selected : 0);

    bool opened = ImGui::TreeNodeEx(
        (void*)(intptr_t)BoneIndex,
        flags,
        "%s",
        *Bones[BoneIndex].Name.ToString()
    );

    // 클릭 시 에디터 컨트롤러에 선택 인덱스 설정
    if (ImGui::IsItemClicked())
    {
        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
        Engine->GetSkeletalMeshEditorController()->SetSelectedBoneIndex(BoneIndex);
        SelectedBoneIndex = BoneIndex;  // 로컬 변수에도 저장해두면 Render() 쪽에서 즉시 반영됩니다.
    }

    if (opened)
    {
        for (int32 ChildIdx : Bones[BoneIndex].Children)
        {
            RenderBoneTree(ChildIdx);
        }
        ImGui::TreePop();
    }
}

void SkeletonMeshEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}
