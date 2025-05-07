#include "SkeletonMeshEditorPanel.h"
#include "UnrealEd/ImGuiWidget.h"
#include "Engine/EditorEngine.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Asset/SkeletalMeshAsset.h"
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

    auto& Bones = SkelMesh->GetRenderData()->Skeleton.Bones;

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

    if (RootIdx != INDEX_NONE)
    {
        ImGui::Text("Bone Hierarchy:");
        ImGui::Separator();
        RenderBoneTree(RootIdx);
    }

    ImGui::Separator();

    // 선택된 본의 FBonePose 편집
    if (SelectedBoneIndex != INDEX_NONE)
    {
        FBone& Bone = Bones[SelectedBoneIndex];
        ImGui::Text("Edit Bone: %s", *Bone.Name.ToString());
        ImGui::Spacing();

        // 현재 로컬 포즈 복사
        FBonePose EditingPose = Bone.LocalTransform;

        // 위치
        float loc[3] = { EditingPose.Location.X, EditingPose.Location.Y, EditingPose.Location.Z };
        if (ImGui::DragFloat3("Location", loc, 0.01f))
        {
            EditingPose.Location = FVector(loc[0], loc[1], loc[2]);
        }

        // 회전 (Euler → Quaternion)
        FRotator rot = EditingPose.Rotation.Rotator();
        float eul[3] = { rot.Pitch, rot.Yaw, rot.Roll };
        if (ImGui::DragFloat3("Rotation (P,Y,R)", eul, 0.5f))
        {
            EditingPose.Rotation = JungleMath::EulerToQuaternion(FVector(eul[0], eul[1], eul[2]));
        }

        // 스케일
        float scl[3] = { EditingPose.Scale.X, EditingPose.Scale.Y, EditingPose.Scale.Z };
        if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.01f, 100.f))
        {
            EditingPose.Scale = FVector(scl[0], scl[1], scl[2]);
        }

        Bone.LocalTransform = EditingPose;
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
    const auto& Bones = SkelMesh->GetRenderData()->Skeleton.Bones;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
        | ImGuiTreeNodeFlags_SpanAvailWidth
        | ((SelectedBoneIndex == BoneIndex) ? ImGuiTreeNodeFlags_Selected : 0);

    // 포인터 대신 unique ID로 노드 구분
    bool opened = ImGui::TreeNodeEx(
        (void*)(intptr_t)BoneIndex,
        flags,
        "%s",
        *Bones[BoneIndex].Name.ToString()
    );

    if (ImGui::IsItemClicked())
    {
        SelectedBoneIndex = BoneIndex;
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
