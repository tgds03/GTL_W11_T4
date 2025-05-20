#include "UnrealEd.h"
#include "EditorPanel.h"

#include "PropertyEditor/ControlEditorPanel.h"
#include "PropertyEditor/OutlinerEditorPanel.h"
#include "PropertyEditor/PropertyEditorPanel.h"
#include "PropertyEditor/SkeletonMeshEditorPanel.h"
#include "PropertyEditor/ParticleEditorPanel.h"

#include "Engine/EditorEngine.h"
#include "Engine/World/World.h"

void UnrealEd::Initialize()
{
    auto ControlPanel = std::make_shared<ControlEditorPanel>();
    ControlPanel->SetVisibleInWorldType(EWorldType::Editor);
    Panels["ControlPanel"] = ControlPanel;
    
    auto OutlinerPanel = std::make_shared<OutlinerEditorPanel>();
    OutlinerPanel->SetVisibleInWorldType(EWorldType::Editor);
    Panels["OutlinerPanel"] = OutlinerPanel;
    
    auto PropertyPanel = std::make_shared<PropertyEditorPanel>();
    PropertyPanel->SetVisibleInWorldType(EWorldType::Editor);
    Panels["PropertyPanel"] = PropertyPanel;

    auto SkeletonPanel = std::make_shared<SkeletonMeshEditorPanel>();
    SkeletonPanel->SetVisibleInWorldType(EWorldType::EditorPreview);
    Panels["SkeletonPanel"] = SkeletonPanel;

    auto ParticlePanel = std::make_shared<ParticleEditorPanel>();
    ParticlePanel->SetVisibleInWorldType(EWorldType::Editor);
    Panels["ParticlePanel"] = ParticlePanel;
}

void UnrealEd::Render() const
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    EWorldType ActiveWorldType = Engine->ActiveWorld->WorldType;
    for (const auto& Panel : Panels)
    {
        // TODO : Panel을 새로 만들지 Render할때 Type을 넣어서 분기 할지 고민
        if (Panel.Key == "ParticlePanel")
        {
            if (bShowParticlePanel)
            {
                if (ActiveWorldType == Panel.Value->VisibleInWorldType ||
                    Panel.Value->VisibleInWorldType == EWorldType::None)
                {
                    Panel.Value->Render();
                }
            }
        }
        else
        {
            if(ActiveWorldType == Panel.Value->VisibleInWorldType ||
                Panel.Value->VisibleInWorldType == EWorldType::None)
            {
                Panel.Value->Render();
            }
        }
    }
}

void UnrealEd::AddEditorPanel(const FString& PanelId, const std::shared_ptr<UEditorPanel>& EditorPanel)
{
    Panels[PanelId] = EditorPanel;
}

void UnrealEd::OnResize(HWND hWnd) const
{
    for (auto& Panel : Panels)
    {
        Panel.Value->OnResize(hWnd);
    }
}

std::shared_ptr<UEditorPanel> UnrealEd::GetEditorPanel(const FString& PanelId)
{  
    return Panels[PanelId];
}
