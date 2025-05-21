#pragma once
#include "Particles/ParticleSystem.h"
#include "UnrealEd/EditorPanel.h"
#include "ThirdParty/include/ImGUI/imgui.h"

class UParticleEmitter;
class UParticleModule;
class FParticlePreviewController;

class FParticleEditorPanel : public FEditorPanel
{
public:
    FParticleEditorPanel();
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    void SetParticlePreviewController(FParticlePreviewController* InController)
    {
        ParticlePreviewController = InController;
    }

    void SetParticleSystem(UParticleSystem* InParticleSystem)
    {
        TargetParticleSystem = InParticleSystem;
    }

private:
    void RenderMenuBar(const ImVec2& InPos, const ImVec2& InSize);
    void RenderToolBar(const ImVec2& InPos, const ImVec2& InSize);
    void RenderViewportPanel(const ImVec2& InPos, const ImVec2& InSize);
    void RenderDetailPanel(const ImVec2& InPos, const ImVec2& InSize);
    void RenderEmitterPanel(const ImVec2& InPos, const ImVec2& InSize);

    void RenderEmitterInfos();
    void RenderDetailInfos();


    UParticleModule* SelectedModule = nullptr;
    UParticleEmitter* SelectedEmitter = nullptr;
    UParticleSystem* TargetParticleSystem = nullptr;
    FParticlePreviewController* ParticlePreviewController = nullptr;
};
