#pragma once
#include "UnrealEd/EditorPanel.h"
#include "ThirdParty/include/ImGUI/imgui.h"

class ParticleEditorPanel : public UEditorPanel
{
public:
    ParticleEditorPanel();
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderMenuBar();
    void RenderToolBar(const ImVec2& InSize);
    void RenderViewportPanel(const ImVec2& InSize);
    void RenderEmitterPanel(const ImVec2& InSize);
    void RenderDetailPanel(const ImVec2& InSize);

    float Width = 1200.f, Height = 700.f;
};
