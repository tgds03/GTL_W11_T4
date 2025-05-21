#include "SkeletonMeshEditorPanel.h"
#include "UnrealEd/ImGuiWidget.h"
#include "Engine/EditorEngine.h"
#include "Components/SkeletalMesh/SkeletalMesh.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"
#include "Math/Quat.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Math/JungleMath.h"
#include "Engine/DataPreviewController.h"

void FSkeletonMeshEditorPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    std::shared_ptr<FDataPreviewController> Controller = Engine->GetSkeletalMeshEditorController();
    EPreviewType PreviewType = Controller->GetType();

    // 1) 메인 뷰포트 정보 가져오기
    const ImGuiViewport* vp = ImGui::GetMainViewport();

    // 2) 다음 Begin() 창의 위치를 뷰포트 왼쪽 상단(WorkPos)으로 고정
    ImGui::SetNextWindowPos(vp->WorkPos, ImGuiCond_Always);

    // 3) 창 너비를 고정(예: 화면 너비의 30%), 높이는 뷰포트 전체 높이로 설정
    float panelWidth = vp->WorkSize.x * 0.2f;
    ImGui::SetNextWindowSize(ImVec2(panelWidth, vp->WorkSize.y - 260), ImGuiCond_Always);

    ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoCollapse);

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

void FSkeletonMeshEditorPanel::RenderCommonEditorHeader()
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

void FSkeletonMeshEditorPanel::RenderSkeletalMeshEditorUI()
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

void FSkeletonMeshEditorPanel::RenderAnimationEditorUI()
{
    UAnimInstance* AnimInstance = Cast<UEditorEngine>(GEngine)
        ->GetSkeletalMeshEditorController()
        ->EdittingAnim;

    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];

    // 패널 설정 - 스크롤 없이 모든 내용이 보이도록 고정 높이
    const float panelHeight = 260.0f;
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, panelHeight), ImGuiCond_Always);
    ImGui::Begin("Animation Editor", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar);

    // 현재 애니메이션 정보 - 애니메이션 시스템에서 실시간으로 가져오기
    float CurrentTime = 0.0f;
    float TotalDuration = 5.0f;
    bool bPlaying = false;
    bool bLooping = true;
    float PlayRate = 1.0f;
    static char AnimNameBuffer[128] = "";

    // 애니메이션 인스턴스가 유효하면 현재 상태 가져오기
    if (AnimInstance && AnimInstance->GetAnimStateMachine() && AnimInstance->GetCurrentSequence())
    {
        // 현재 시간 가져오기 - 이 부분이 중요! 매 프레임 업데이트
        CurrentTime = AnimInstance->GetCurrentSequence()->GetLocalTime();

        // 애니메이션 총 길이
        TotalDuration = AnimInstance->GetCurrentSequence()->GetUnScaledPlayLength();

        // 재생 중인지 확인
        bPlaying = AnimInstance->GetIsPlaying();
        // 루프 상태
        bLooping = AnimInstance->GetCurrentSequence()->IsLooping();

        // 재생 속도
        PlayRate = AnimInstance->GetCurrentSequence()->GetRateScale();

        // 애니메이션 이름 (처음 한 번만 가져옴)
        if (AnimNameBuffer[0] == '\0')
        {
            //FString AnimName = AnimInstance->GetAnimStateMachine()->GetCurrentSequence()->GetName();
            //FCString::Strncpy(AnimNameBuffer, *AnimName, IM_ARRAYSIZE(AnimNameBuffer));
        }
    }

    // === 1. 상단 컨트롤 바 ===
    ImGui::BeginGroup();
    {
        // 애니메이션 이름 및 로드 버튼
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Animation:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##AnimName", AnimNameBuffer, IM_ARRAYSIZE(AnimNameBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            FString FileName(AnimNameBuffer);
            if (FileName.Len() == 0)
            {
                FileName = "Twerkbin";
            }
            FString FbxPath(TEXT("Contents/Fbx/") + FileName + TEXT(".fbx"));
            UAnimSequence* AnimSequence = FResourceManager::LoadAnimationSequence(FbxPath);
            if (!AnimSequence)
            {
                UE_LOG(LogLevel::Warning, TEXT("애니메이션 로드 실패, 스켈레톤만 표시합니다."));
                return;
            }

            //// 애니메이션 로드 성공 후 초기화
            //if (AnimInstance && AnimInstance->GetAnimStateMachine())
            //{
            //    AnimInstance->StartAnimSequence(AnimSequence, 0.3f);
            //    CurrentTime = 0.0f;
            //    TotalDuration = AnimSequence->GetUnScaledPlayLength();
            //}
        }

        // 재생 컨트롤 버튼
        ImGui::SameLine(0, 20);
        ImGui::PushFont(IconFont);

        // 재생/일시정지 토글 버튼
        if (ImGui::Button(bPlaying ? "\ue9a8" : "\uf04b", ImVec2(28, 28))) {
            bPlaying = !bPlaying;

            if (AnimInstance)
            {
                AnimInstance->SetIsPlaying(bPlaying);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("\ue9e4", ImVec2(28, 28))) {
            // 애니메이션 정지 및 리셋
            if (AnimInstance && AnimInstance->GetAnimStateMachine())
            {
                AnimInstance->GetCurrentSequence()->SetLocalTime(0.0f);
                CurrentTime = 0.0f;
                bPlaying = false;
                AnimInstance->GetCurrentSequence()->SetLocalTime(CurrentTime);
                AnimInstance->SetIsPlaying(bPlaying);
            }
        }
        ImGui::PopFont();

        // 루핑 설정
        ImGui::SameLine(0, 15);
        if (ImGui::Checkbox("Loop", &bLooping)) {
            if (AnimInstance && AnimInstance->GetAnimStateMachine() &&
                AnimInstance->GetCurrentSequence())
            {
                AnimInstance->GetCurrentSequence()->SetLooping(bLooping);
            }
        }

        // 재생 속도 설정
        ImGui::SameLine(0, 15);
        ImGui::Text("Speed:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputFloat("##PlayRate", &PlayRate, 0.1f, 0.5f, "%.1f")) {
            PlayRate = FMath::Clamp(PlayRate, -10.0f, 10.0f);

            if (AnimInstance && AnimInstance->GetAnimStateMachine() &&
                AnimInstance->GetCurrentSequence())
            {
                AnimInstance->GetCurrentSequence()->SetRateScale(PlayRate);
            }
        }

        // 현재 시간/총 시간 표시
        ImGui::SameLine(0, 20);
        ImGui::Text("Time: %.2f / %.2f sec", CurrentTime, TotalDuration);
    }
    ImGui::EndGroup();

    ImGui::Separator();

    // === 2. 메인 영역 - 타임라인과 노티파이 목록 ===
    const float leftColumnWidth = ImGui::GetContentRegionAvail().x * 0.7f;

    // 노티파이 편집 관련 변수 (클래스 멤버 변수로 옮기는 것이 좋음)
    static bool bEditingNotify = false;
    static int EditingNotifyIndex = -1;
    static char EditNotifyName[64] = "";
    static float EditNotifyTime = 0.0f;
    static bool EditNotifyModalOpen = false;

    // 왼쪽 컬럼: 타임라인
    ImGui::BeginChild("LeftPanel", ImVec2(leftColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
    {
        // 타임라인 슬라이더
        if (ImGui::SliderFloat("##Timeline", &CurrentTime, 0.0f, TotalDuration, "")) {
            // 슬라이더로 시간 변경 시 애니메이션 시간도 설정
            if (AnimInstance && AnimInstance->GetAnimStateMachine())
            {
                AnimInstance->GetCurrentSequence()->SetLocalTime(CurrentTime);
            }
        }

        // 타임라인 시각화 영역
        const float timelineHeight = 50.0f;
        ImVec2 timelineStart = ImGui::GetCursorScreenPos();
        ImVec2 timelineEnd = ImVec2(timelineStart.x + leftColumnWidth - 20, timelineStart.y + timelineHeight);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 타임라인 배경
        drawList->AddRectFilled(timelineStart, timelineEnd, IM_COL32(40, 40, 40, 255));

        // 눈금 표시
        int numTicks = 10;
        for (int i = 0; i <= numTicks; i++) {
            float tickPos = timelineStart.x + (timelineEnd.x - timelineStart.x) * ((float)i / numTicks);
            float tickHeight = (i % 5 == 0) ? timelineHeight * 0.3f : timelineHeight * 0.15f;

            // 눈금 선
            drawList->AddLine(
                ImVec2(tickPos, timelineEnd.y - 2),
                ImVec2(tickPos, timelineEnd.y - tickHeight - 2),
                IM_COL32(200, 200, 200, 255),
                1.0f
            );

            // 주요 눈금에 시간 표시
            if (i % 5 == 0) {
                char tickLabel[16];
                snprintf(tickLabel, sizeof(tickLabel), "%.1f", (TotalDuration * i) / numTicks);
                drawList->AddText(
                    ImVec2(tickPos - 10, timelineEnd.y - tickHeight - 15),
                    IM_COL32(200, 200, 200, 255),
                    tickLabel
                );
            }
        }

        // 노티파이 마커 표시
        TArray<FAnimNotifyEvent> Notifies;

        // 실제 노티파이 가져오기
        if (AnimInstance && AnimInstance->GetAnimStateMachine() &&
            AnimInstance->GetCurrentSequence())
        {
            Notifies = AnimInstance->GetCurrentSequence()->GetNotifies();
        }
        else
        {
            // 테스트용 데이터
            static struct { float Time; const char* Name; }
            TestNotifies[] = { {1.0f, "FootStep"}, {2.5f, "Attack"} };

            for (int i = 0; i < IM_ARRAYSIZE(TestNotifies); i++)
            {
                FAnimNotifyEvent Notify;
                Notify.TriggerTime = TestNotifies[i].Time / TotalDuration;
                Notify.NotifyName = FName(TestNotifies[i].Name);
                Notifies.Add(Notify);
            }
        }

        // 노티파이 표시
        for (int i = 0; i < Notifies.Num(); i++) {
            float NotifyTime = Notifies[i].TriggerTime * TotalDuration;
            FName NotifyName = Notifies[i].NotifyName;

            float notifyPos = timelineStart.x + (timelineEnd.x - timelineStart.x) * (NotifyTime / TotalDuration);

            // 마커 그리기 (삼각형)
            ImVec2 markerPoints[3] = {
                ImVec2(notifyPos, timelineStart.y + 4),
                ImVec2(notifyPos - 6, timelineStart.y + 12),
                ImVec2(notifyPos + 6, timelineStart.y + 12)
            };

            // 노티파이 마커 색상
            ImU32 markerColor = IM_COL32(0, 200, 0, 255);
            if (NotifyName.ToString() == "Attack") {
                markerColor = IM_COL32(200, 0, 0, 255);
            }

            drawList->AddConvexPolyFilled(markerPoints, 3, markerColor);

            // 노티파이 이름 표시
            drawList->AddText(
                ImVec2(notifyPos - 20, timelineStart.y + 14),
                IM_COL32(200, 200, 200, 255),
                *NotifyName.ToString()
            );

            // 마커 클릭 처리
            ImVec2 markerRect1(notifyPos - 8, timelineStart.y);
            ImVec2 markerRect2(notifyPos + 8, timelineStart.y + 16);
            if (ImGui::IsMouseHoveringRect(markerRect1, markerRect2)) {
                // 마우스 호버 시 툴팁
                ImGui::BeginTooltip();
                ImGui::Text("%s (%.2f sec)", *NotifyName.ToString(), NotifyTime);
                ImGui::EndTooltip();

                // 클릭 시 해당 시간으로 이동
                if (ImGui::IsMouseClicked(0)) {
                    CurrentTime = NotifyTime;
                    if (AnimInstance && AnimInstance->GetAnimStateMachine())
                    {
                        AnimInstance->GetCurrentSequence()->SetLocalTime(CurrentTime);
                    }
                }

                // 우클릭 시 컨텍스트 메뉴
                if (ImGui::IsMouseClicked(1)) {
                    ImGui::OpenPopup(("NotifyContextMenu_" + std::to_string(i)).c_str());
                }
            }

            // 컨텍스트 메뉴 처리
            if (ImGui::BeginPopup(("NotifyContextMenu_" + std::to_string(i)).c_str())) {
                if (ImGui::MenuItem("Jump to")) {
                    CurrentTime = NotifyTime;
                    if (AnimInstance && AnimInstance->GetAnimStateMachine())
                    {
                        AnimInstance->GetCurrentSequence()->SetLocalTime(CurrentTime);
                    }
                }

                // Edit 메뉴 항목 처리 - 편집 상태만 설정하고 팝업은 나중에 열기
                if (ImGui::MenuItem("Edit")) {
                    if (AnimInstance && AnimInstance->GetCurrentSequence()) {
                        // 편집할 노티파이 정보 저장
                        EditingNotifyIndex = i;
                        strcpy_s(EditNotifyName, *NotifyName.ToString());
                        EditNotifyTime = NotifyTime;
                        EditNotifyModalOpen = true;
                    }
                }

                if (ImGui::MenuItem("Delete")) {
                    if (AnimInstance && AnimInstance->GetAnimStateMachine() &&
                        AnimInstance->GetCurrentSequence())
                    {
                        AnimInstance->GetCurrentSequence()->RemoveNotify(i);
                    }
                }

                ImGui::EndPopup();
            }
        }

        // 현재 시간 표시 (세로선) - 이 부분이 동영상 진행바처럼 움직여야 함
        float currentTimePos = timelineStart.x + (timelineEnd.x - timelineStart.x) * (CurrentTime / TotalDuration);
        drawList->AddLine(
            ImVec2(currentTimePos, timelineStart.y),
            ImVec2(currentTimePos, timelineEnd.y),
            IM_COL32(255, 50, 50, 255),
            2.0f
        );

        // 타임라인 클릭/드래그 처리
        ImGui::SetCursorScreenPos(timelineStart);
        ImGui::InvisibleButton("TimelineDrag", ImVec2(timelineEnd.x - timelineStart.x, timelineHeight));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
            float mouseX = ImGui::GetIO().MousePos.x;
            float relativePos = (mouseX - timelineStart.x) / (timelineEnd.x - timelineStart.x);
            relativePos = FMath::Clamp(relativePos, 0.0f, 1.0f);

            CurrentTime = relativePos * TotalDuration;

            if (AnimInstance && AnimInstance->GetAnimStateMachine())
            {
                AnimInstance->GetCurrentSequence()->SetLocalTime(CurrentTime);
            }
        }

        // 빈 공간 추가
        ImGui::Dummy(ImVec2(0, timelineHeight + 5));
    }
    ImGui::EndChild();

    // 오른쪽 컬럼: 노티파이 관리
    ImGui::SameLine();
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    {
        ImGui::Text("Notify Events");
        ImGui::Separator();

        // 노티파이 추가 컨트롤
        static char NewNotifyName[64] = "NewNotify";
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 60);
        ImGui::InputText("##NotifyName", NewNotifyName, IM_ARRAYSIZE(NewNotifyName));
        ImGui::SameLine();
        if (ImGui::Button("Add")) {
            if (AnimInstance && AnimInstance->GetCurrentSequence()) {
                // 현재 시간 기준으로 노티파이 추가
                float NormalizedTime = CurrentTime / TotalDuration;

                // 노티파이 생성 및 추가
                FAnimNotifyEvent NewNotify;
                NewNotify.TriggerTime = NormalizedTime;
                NewNotify.NotifyName = FName(NewNotifyName);

                AnimInstance->GetCurrentSequence()->AddNotify(NewNotify);
            }
        }

        // 노티파이 목록 표시
        if (ImGui::BeginTable("NotifyTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableHeadersRow();

            // 노티파이 목록 표시
            if (AnimInstance && AnimInstance->GetCurrentSequence()) {
                TArray<FAnimNotifyEvent> Notifies = AnimInstance->GetCurrentSequence()->GetNotifies();

                for (int i = 0; i < Notifies.Num(); i++) {
                    float NotifyTime = Notifies[i].TriggerTime * TotalDuration;
                    FName NotifyName = Notifies[i].NotifyName;

                    ImGui::TableNextRow();

                    // 이름 열
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", *NotifyName.ToString());

                    // 시간 열
                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f sec", NotifyTime);

                    // 액션 열
                    ImGui::TableNextColumn();
                    ImGui::PushID(i);

                    // 이동 버튼
                    if (ImGui::Button("Go")) {
                        CurrentTime = NotifyTime;
                        if (AnimInstance) {
                            AnimInstance->GetCurrentSequence()->SetLocalTime(CurrentTime);
                        }
                    }

                    ImGui::SameLine();

                    // 삭제 버튼
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                    if (ImGui::Button("X")) {
                        if (AnimInstance && AnimInstance->GetCurrentSequence()) {
                            AnimInstance->GetCurrentSequence()->RemoveNotify(i);
                        }
                    }
                    ImGui::PopStyleColor();

                    ImGui::PopID();
                }
            }

            ImGui::EndTable();
        }
    }
    ImGui::EndChild();

    // 노티파이 편집 모달 팝업 (메인 윈도우 레벨에서 처리)
    if (EditNotifyModalOpen) {
        ImGui::OpenPopup("Edit Notify Modal");
    }

    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    if (ImGui::BeginPopupModal("Edit Notify Modal", &EditNotifyModalOpen)) {
        ImGui::Text("Edit Notify Event");
        ImGui::Separator();

        // 노티파이 이름 편집
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##EditNotifyName", EditNotifyName, IM_ARRAYSIZE(EditNotifyName));

        // 노티파이 시간 편집
        ImGui::Text("Time (seconds):");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderFloat("##EditNotifyTime", &EditNotifyTime, 0.0f, TotalDuration, "%.3f");

        ImGui::Separator();

        // 버튼 레이아웃을 위한 공간
        float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2;

        // 저장 버튼
        if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
            // 노티파이 업데이트
            if (AnimInstance && AnimInstance->GetCurrentSequence() &&
                EditingNotifyIndex >= 0 && EditingNotifyIndex < AnimInstance->GetCurrentSequence()->GetNotifies().Num()) {


                // 새 노티파이 정보 생성
                FAnimNotifyEvent UpdatedNotify;
                UpdatedNotify.NotifyName = FName(EditNotifyName);
                UpdatedNotify.TriggerTime = EditNotifyTime / TotalDuration; // 정규화된 시간으로 변환

                // 기존 노티파이 제거 후 업데이트된 노티파이 추가
     
                AnimInstance->GetCurrentSequence()->RemoveNotify(EditingNotifyIndex);
                AnimInstance->GetCurrentSequence()->Notifies.Add(UpdatedNotify);
          
            }

            // 모달 닫기
            EditNotifyModalOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // 취소 버튼
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            EditNotifyModalOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // 디버그 정보 표시 (필요시 활성화)
    //ImGui::Text("DEBUG: EditNotifyModalOpen=%s, EditingNotifyIndex=%d", 
    //    EditNotifyModalOpen ? "true" : "false", EditingNotifyIndex);

    ImGui::End();
}

void FSkeletonMeshEditorPanel::RenderBoneTree(int32 BoneIndex)
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

USkeletalMesh* FSkeletonMeshEditorPanel::GetCurrentEdittingMesh() const
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return nullptr;

    return Engine->GetSkeletalMeshEditorController()->OriginalMesh;
}

int32 FSkeletonMeshEditorPanel::GetRootBoneIndex(USkeletalMesh* Mesh) const
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

void FSkeletonMeshEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}


