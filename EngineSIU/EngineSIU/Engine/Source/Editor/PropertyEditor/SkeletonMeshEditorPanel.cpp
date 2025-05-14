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
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    std::shared_ptr<UDataPreviewController> Controller = Engine->GetSkeletalMeshEditorController();
    EPreviewType PreviewType = Controller->GetType();

    ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    RenderCommonEditorHeader();

    switch (PreviewType)
    {
    case EPreviewType::SkeletalMesh:
        RenderSkeletalMeshEditorUI();
        break;

    case EPreviewType::Animation:
        RenderSkeletalMeshEditorUI();
        RenderAnimationEditorUI();
        break;

    default:
        ImGui::Text("Unsupported preview type.");
        break;
    }

    ImGui::End();
}

void SkeletonMeshEditorPanel::RenderCommonEditorHeader()
{
    if (ImGui::Button("Exit Edit Mode"))
    {
        if (UEditorEngine* Engine = Cast<UEditorEngine>(GEngine))
        {
            Engine->EndEditorPreviewMode();
        }
    }

    ImGui::Separator();
}

void SkeletonMeshEditorPanel::RenderSkeletalMeshEditorUI()
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
}

void SkeletonMeshEditorPanel::RenderAnimationEditorUI()
{
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];

    // 새로운 독립된 창 시작
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 300), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 300), ImGuiCond_Always);

    // 높이 조절 가능하고 좌우는 화면 꽉 차도록
    ImGui::Begin("Animation Editor", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // === 1. 좌측 상단: Anim Name + Load 버튼 ===
    ImGui::BeginGroup();
    static char AnimNameBuffer[128] = "";

    // AnimName Label + Input
    ImGui::Text("Animation Name ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("##Anim Name", AnimNameBuffer, IM_ARRAYSIZE(AnimNameBuffer));
    ImGui::SameLine(0.0f, 10.0f); // Load 버튼과 간격

    // Load 버튼
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.2f, 1.0f));
    ImGui::Button("Load");
    ImGui::PopStyleColor(3);

    // 아이콘 버튼들
    ImGui::SameLine(0.0f, 50.0f);
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9a8", ImVec2(32, 32))) // Play
    {
        UE_LOG(LogLevel::Display, TEXT("Animation Play"));
    }
    ImGui::SameLine(0.0f, 5.0f);
    if (ImGui::Button("\ue99c", ImVec2(32, 32))) // Pause
    {
        UE_LOG(LogLevel::Display, TEXT("Animation Pause"));
    }
    ImGui::SameLine(0.0f, 5.0f);
    if (ImGui::Button("\ue9e4", ImVec2(32, 32))) // Stop
    {
        UE_LOG(LogLevel::Display, TEXT("Animation Stop"));
    }
    ImGui::PopFont();

    ImGui::SameLine(0.0f, 50.0f);
    static bool bLooping = true;
    ImGui::Checkbox("Looping", &bLooping);  // ← 체크박스 추가


    // PlayRate 입력
    ImGui::SameLine(0.0f, 50.0f);
    static float PlayRate = 1.0f;
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputFloat("##PlayRate", &PlayRate, 0.1f, 1.0f, "%.2f");
    ImGui::EndGroup();

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

USkeletalMesh* SkeletonMeshEditorPanel::GetCurrentEdittingMesh() const
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return nullptr;

    return Engine->GetSkeletalMeshEditorController()->OriginalMesh;
}

int32 SkeletonMeshEditorPanel::GetRootBoneIndex(USkeletalMesh* Mesh) const
{
    if (!Mesh)
        return INDEX_NONE;

    const TArray<FBone>& Bones = Mesh->GetSkeleton()->Bones;

    for (int32 i = 0; i < Bones.Num(); ++i)
    {
        if (Bones[i].ParentIndex == INDEX_NONE)
        {
            return i;
        }
    }

    return INDEX_NONE;
}

void SkeletonMeshEditorPanel::RenderSequenceUI()
{
    //UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    //auto Controller = Engine->GetSkeletalMeshEditorController();
    //UAnimInstance* Anim = Controller->EditingAnim;

    //if (!Anim)
    //{
    //    ImGui::Text("No animation loaded.");
    //    return;
    //}

    //ImGui::Text("Animation Sequences:");

    //for (const auto& Pair : Anim->GetAnimSequenceMap())
    //{
    //    FString StateName = TEXT("Unknown");
    //    switch (Pair.Key)
    //    {
    //    case EAnimState::Idle: StateName = TEXT("Idle"); break;
    //    case EAnimState::Twerk: StateName = TEXT("Twerk"); break;
    //        // 필요 시 추가
    //    }

    //    ImGui::Text(" - %s", *StateName);
    //}

    //ImGui::Separator();

    //ImGui::Text("Current Sequence Time: %.3f", Anim->GetCurrentTime());
    //ImGui::Text("Play Rate: %.2fx", Anim->GetPlayRate());
    //ImGui::Checkbox("Is Playing", &Anim->IsPlaying());

    //if (ImGui::Button("Play"))
    //{
    //    Anim->SetPlaying(true);
    //}

    //ImGui::SameLine();
    //if (ImGui::Button("Pause"))
    //{
    //    Anim->SetPlaying(false);
    //}

    //ImGui::SameLine();
    //if (ImGui::Button("Reset"))
    //{
    //    Anim->SetCurrentTime(0.f);
    //}
}

void SkeletonMeshEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}


