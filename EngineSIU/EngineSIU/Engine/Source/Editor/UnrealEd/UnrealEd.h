#pragma once
#include "Container/Map.h"
#include "Container/String.h"

class FEditorPanel;

class UnrealEd
{
public:
    UnrealEd() = default;
    ~UnrealEd() = default;
    void Initialize();
    
     void Render() const;
     void OnResize(HWND hWnd) const;
    
    void AddEditorPanel(const FString& PanelId, const std::shared_ptr<FEditorPanel>& EditorPanel);
    std::shared_ptr<FEditorPanel> GetEditorPanel(const FString& PanelId);

    bool bShowParticlePanel = true;

private:
    TMap<FString, std::shared_ptr<FEditorPanel>> Panels;
};
