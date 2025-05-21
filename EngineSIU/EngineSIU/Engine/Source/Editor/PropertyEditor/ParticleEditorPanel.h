#pragma once
#include "Particles/ParticleSystem.h"
#include "UnrealEd/EditorPanel.h"
#include "ThirdParty/include/ImGUI/imgui.h"

class UParticleModule;

class FParticleEditorPanel : public FEditorPanel
{
public:
    FParticleEditorPanel();
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderMenuBar(const ImVec2& InPos, const ImVec2& InSize);
    void RenderToolBar(const ImVec2& InPos, const ImVec2& InSize, ImFont* IconFont);
    void RenderViewportPanel(const ImVec2& InPos, const ImVec2& InSize);
    void RenderDetailPanel(const ImVec2& InPos, const ImVec2& InSize);
    void RenderEmitterPanel(const ImVec2& InPos, const ImVec2& InSize);

    void RenderEmitterInfos();
    void RenderDetailInfos();

    void CalculatePanelSize(RECT InRect);

    UParticleModule* SelectedModule = nullptr;
    UParticleSystem* TargetParticleSystem = nullptr;
    ImVec4 BackgroundColor;
    ImVec2 IconSize;

private:
    // Begin Panel Size
    float WinWidth;
    float MenuBarHeight;
    float ToolBarHeight;

    float EmitterPanelXPos;
    float UsableScreenYOffset;

    float EmitterPanelWidth;
    float UsableScreenHeight;

    float DetailPanelYPos;

    float LeftAreaWidth;
    float DetailPanelHeight;

    float ViewportPanelHeight;
    // ~End Panel Size

};
