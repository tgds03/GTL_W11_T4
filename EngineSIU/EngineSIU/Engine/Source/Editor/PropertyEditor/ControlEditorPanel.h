#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"
#include "LightGridGenerator.h"

class FControlEditorPanel : public FEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont);
    static void CreateFlagButton();
    static void CreatePIEButton(ImVec2 ButtonSize, ImFont* IconFont);
    static void CreateSRTButton(ImVec2 ButtonSize);
    void CreateLightSpawnButton(ImVec2 InButtonSize, ImFont* IconFont);
    
private:
    float Width = 300, Height = 100;
    bool bOpenMenu = false;
    bool bShowImGuiDemoWindow = false; // 데모 창 표시 여부를 관리하는 변수
    bool bShowParticleEditorWindow = false; // 파티클 에디터 창 표시 여부를 관리하는 변수

    float* FOV = nullptr;
    float CameraSpeed = 0.0f;
    float GridScale = 1.0f;
    FLightGridGenerator LightGridGenerator;
};

