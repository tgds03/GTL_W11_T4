#include "ParticleEditorPanel.h"

ParticleEditorPanel::ParticleEditorPanel()
{
    // 초기화 필요시 작성
}

void ParticleEditorPanel::Render()
{
    ImVec2 EditorWindowSize(Width, Height);
    ImGui::SetNextWindowSize(EditorWindowSize, ImGuiCond_FirstUseEver);
    constexpr ImGuiWindowFlags EditorFlags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.0f));
    ImGui::Begin("Particle Editor", nullptr, EditorFlags);

    RenderMenuBar();
    RenderToolBar(ImVec2(EditorWindowSize.x, 40));

    ImGui::BeginChild("MainArea", ImVec2(0, 0), false);

    // 왼쪽: 뷰포트(상), 디테일(하)
    ImGui::BeginChild("LeftPanel", ImVec2(600, 0), false);
    RenderViewportPanel(ImVec2(0, 400));
    RenderDetailPanel(ImVec2(0, 0));
    ImGui::EndChild();

    ImGui::SameLine();

    // 오른쪽: 이미터 패널
    ImGui::BeginChild("RightPanel", ImVec2(580, 0), false);
    RenderEmitterPanel(ImVec2(0, 0));
    ImGui::EndChild();

    ImGui::EndChild(); // MainArea

    ImGui::End();
    ImGui::PopStyleColor();
}

void ParticleEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void ParticleEditorPanel::RenderMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
        if (ImGui::BeginMenu("Edit")) { ImGui::EndMenu(); }
        if (ImGui::BeginMenu("Asset")) { ImGui::EndMenu(); }
        if (ImGui::BeginMenu("Window")) { ImGui::EndMenu(); }
        if (ImGui::BeginMenu("Help")) { ImGui::EndMenu(); }
        ImGui::EndMenuBar();
    }
}

void ParticleEditorPanel::RenderToolBar(const ImVec2& InSize)
{
    ImGui::BeginChild("Toolbar", InSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::EndChild();
}

void ParticleEditorPanel::RenderViewportPanel(const ImVec2& InSize)
{
    ImGui::BeginChild("ViewportPanel", InSize, true);

    // 뷰포트 타이틀 
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), " Preview");
    ImGui::Separator();

    ImGui::EndChild();
}

void ParticleEditorPanel::RenderDetailPanel(const ImVec2& InSize)
{
    ImGui::BeginChild("DetailPanel", InSize, true);
    
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), " Properties");
    ImGui::Separator();

    ImGui::EndChild();
}

void ParticleEditorPanel::RenderEmitterPanel(const ImVec2& InSize)
{
    ImGui::BeginChild("EmitterPanel", InSize, true);
    
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), " Emitters");
    ImGui::Separator();

    ImGui::EndChild();
}
