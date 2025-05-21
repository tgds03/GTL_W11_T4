#include "ParticleEditorPanel.h"
#include "Engine/EditorEngine.h"

#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModule.h"
#include "Particles/ParticleModuleColor.h"
#include "Particles/ParticleModuleLifetime.h"
#include "Particles/ParticleModuleLocation.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleModuleSize.h"
#include "Particles/ParticleModuleSpawn.h"
#include "Particles/ParticleModuleSubUV.h"
#include "Particles/ParticleSystem.h"
#include "UObject/ObjectFactory.h"

#include "Particles/ParticleModuleVelocity.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"
#include "Font/IconDefs.h"
#include "Particles/ParticleSpriteEmitter.h"

#include "Classes/Engine/ParticlePreviewController.h"
#include "Editor/UnrealEd/EditorViewportClient.h"
#include "Engine/UnrealClient.h"
#include <array>

FParticleEditorPanel::FParticleEditorPanel()
{
    // 초기화 필요시 작성
    BackgroundColor = ImVec4(0.01f, 0.01f, 0.01f, 1.f);
}

void FParticleEditorPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    

    HWND Hwnd = ::GetActiveWindow();
    RECT ClientRect = { 0,0,0,0 };
    if (Hwnd)
    {
        GetClientRect(Hwnd, &ClientRect);
    }

    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);

    CalculatePanelSize(ClientRect);

    RenderMenuBar(ImVec2(0.0f, 0.0f), ImVec2(WinWidth, MenuBarHeight));
    RenderToolBar(ImVec2(0.0f, MenuBarHeight), ImVec2(WinWidth, ToolBarHeight), IconFont);
    RenderEmitterPanel(ImVec2(EmitterPanelXPos, UsableScreenYOffset), ImVec2(EmitterPanelWidth, UsableScreenHeight));
    RenderDetailPanel(ImVec2(0.0f, DetailPanelYPos), ImVec2(LeftAreaWidth, DetailPanelHeight));
    RenderViewportPanel(ImVec2(0.0f, UsableScreenYOffset), ImVec2(LeftAreaWidth, ViewportPanelHeight));
}

void FParticleEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
}

void FParticleEditorPanel::RenderMenuBar(const ImVec2& InPos, const ImVec2& InSize)
{
    ImGui::SetNextWindowPos(InPos);
    ImGui::SetNextWindowSize(InSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 1.f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);

    ImGui::Begin("MenuBar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_MenuBar
    );
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Particle System")) { /* TODO */ }
            if (ImGui::MenuItem("Open Particle System...", "Ctrl+O")) { /* TODO */ }
            if (ImGui::MenuItem("Save Particle System", "Ctrl+S")) { /* TODO */ }
            if (ImGui::MenuItem("Save As...")) { /* TODO */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) 
            {
                UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
                Engine->EndParticlePreviewMode();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) { /* TODO */ }
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) { /* TODO: Implement */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) { /* TODO */ }
            if (ImGui::MenuItem("Copy", "CTRL+C")) { /* TODO */ }
            if (ImGui::MenuItem("Paste", "CTRL+V")) { /* TODO */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            // Example: Toggle panels
            // static bool show_emitter_panel = true; ImGui::MenuItem("Emitter Panel", NULL, &show_emitter_panel);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }


    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void FParticleEditorPanel::RenderToolBar(const ImVec2& InPos, const ImVec2& InSize, ImFont* IconFont)
{
    ImGui::SetNextWindowPos(InPos);
    ImGui::SetNextWindowSize(InSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, BackgroundColor);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);

    ImGui::Begin("ToolBar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse
    );

    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9d6", IconSize)) {/* TODO: Save logic */ }\
    ImGui::SameLine();
    if (ImGui::Button("\ue9d8", IconSize)) { /* TODO: Play particle system */ }
    ImGui::SameLine();
    ImGui::PopFont();


    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void FParticleEditorPanel::RenderViewportPanel(const ImVec2& InPos, const ImVec2& InSize)
{
    ImGui::SetNextWindowPos(InPos);
    ImGui::SetNextWindowSize(InSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar
    );
    FViewportResource* ViewportResource = ParticlePreviewController->GetViewportClient()->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(EResourceType::ERT_Compositing);

    ID3D11ShaderResourceView* SRV = RenderTargetRHI->SRV;
    ImGui::Image(reinterpret_cast<ImTextureID>(SRV),InSize);

    ImGui::SameLine();


    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void FParticleEditorPanel::RenderDetailPanel(const ImVec2& InPos, const ImVec2& InSize)
{
    ImGui::SetNextWindowPos(InPos);
    ImGui::SetNextWindowSize(InSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, BackgroundColor);
    ImGui::Begin("Properties", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_HorizontalScrollbar
    );

    RenderDetailInfos();

    ImGui::End();
    ImGui::PopStyleColor();
}

void FParticleEditorPanel::RenderEmitterPanel(const ImVec2& InPos, const ImVec2& InSize)
{
    ImGui::SetNextWindowPos(InPos);
    ImGui::SetNextWindowSize(InSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 1.f));
    ImGui::Begin("Emitters", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize
    );

    RenderEmitterInfos();

    ImGui::End();
    ImGui::PopStyleColor();
}

void FParticleEditorPanel::RenderEmitterInfos()
{
    if (!TargetParticleSystem)
    {
        ImGui::Text("No Particle System loaded.");
        return;
    }

    if (ImGui::Button("Add Emitter"))
    {
        UParticleSpriteEmitter* ParticleSpriteEmitter = FObjectFactory::ConstructObject<UParticleSpriteEmitter>(nullptr);
        TargetParticleSystem->Emitters.Add(ParticleSpriteEmitter);
        ParticleSpriteEmitter->CreateLODLevel(0);
        ParticleSpriteEmitter->SetToSensibleDefaults();
    }
    ImGui::Separator();

    ImGui::BeginChild("EmittersHorizontalArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    bool bIsFirstEmitter = true;
    const float emitterColumnWidth = 300.f;
    const float modulesListHeight = 800.f;
    for (int i = 0; i < TargetParticleSystem->Emitters.Num(); ++i)
    {
        UParticleEmitter* Emitter = TargetParticleSystem->Emitters[i];
        if (!Emitter) continue;

        if (!bIsFirstEmitter)
        {
            ImGui::SameLine();
        }
        bIsFirstEmitter = false;

        ImGui::PushID(Emitter);

        ImGui::BeginChild(ImGui::GetID("EmitterColumn"), ImVec2(emitterColumnWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
        {
            FString EmitterName = FString::Printf(TEXT("Emitter %d"), i);

            if (ImGui::CollapsingHeader((*EmitterName), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::BeginChild("ModulesList", ImVec2(0, modulesListHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
                if (Emitter->LODLevels.Num() > 0 && Emitter->LODLevels[0])
                {
                    TArray<UParticleModule*>& Modules = Emitter->LODLevels[0]->Modules;
                    for (int j = 0; j < Modules.Num(); ++j)
                    {
                        UParticleModule* Module = Modules[j];
                        if (!Module) continue;

                        bool bSelected = (Module == SelectedModule);
                        FString ModuleDisplayName = Module->GetClass()->GetName();
                        // ModuleDisplayName.RemoveFromStart(TEXT("ParticleModule"));

                        if (ImGui::Selectable((*ModuleDisplayName), bSelected, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAllColumns))
                        {
                            SelectedModule = Module;
                        }
                    }
                }
                ImGui::EndChild();
            }
        }

        ImGui::EndChild();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void FParticleEditorPanel::RenderDetailInfos()
{
    if (SelectedModule == nullptr)
    {
        ImGui::TextWrapped("Select a module from an emitter to see its properties.");
        return;
    }

    FString ModuleClassName = SelectedModule->GetClass()->GetName();
    FString ModuleDisplayName = ModuleClassName;
    //ModuleDisplayName.RemoveFromStart(TEXT("ParticleModule"));

    ImGui::Text("Properties: %s", (*ModuleDisplayName));
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGuiTreeNodeFlags TreeNodePropertyFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;


    if (UParticleModuleRequired* RequiredModule = Cast<UParticleModuleRequired>(SelectedModule))
    {
        if (ImGui::TreeNodeEx("Transform", TreeNodePropertyFlags))
        {

            FVector Origin = RequiredModule->EmitterOrigin;
            FImGuiWidget::DrawVec3Control("Emitter Origin", Origin, 0.0f, 100.0f);
            RequiredModule->EmitterOrigin = Origin;
            ImGui::Spacing();

            FRotator Rotation = RequiredModule->EmitterRotation;
            FVector RotVector = Rotation.ToVector();
            FImGuiWidget::DrawVec3Control("Emitter Rotation", RotVector, 0.0f, 100.0f);
            RequiredModule->EmitterRotation = Rotation;
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Behavior", TreeNodePropertyFlags))
        {
            bool bUseLocalSpace = RequiredModule->bUseLocalSpace;
            if (ImGui::Checkbox("Use Local Space", &bUseLocalSpace))
            {
                RequiredModule->bUseLocalSpace = bUseLocalSpace;
            }

            bool bKillOnDeactive = RequiredModule->bKillOnDeactivate;
            if (ImGui::Checkbox("Kill On Deactivate", &bKillOnDeactive))
            {
                RequiredModule->bKillOnDeactivate = bKillOnDeactive;
            }

            bool bKillOnCompleted = RequiredModule->bKillOnCompleted;
            if (ImGui::Checkbox("Kill On Completed", &bKillOnCompleted))
            {
                RequiredModule->bKillOnCompleted = bKillOnCompleted;
            }

            const char* SortModeLabels[] = { "None", "ViewProjDepth", "DistanceToView", "Age_OldestFirst", "Age_NewestFirst" };
            int CurrentSortMode = static_cast<int>(RequiredModule->SortMode);
            if (ImGui::Combo("Sorting Mode", &CurrentSortMode, SortModeLabels, IM_ARRAYSIZE(SortModeLabels)))
            {
                RequiredModule->SortMode = TEnumAsByte<EParticleSortMode>(static_cast<uint8>(CurrentSortMode));
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Timing", TreeNodePropertyFlags))
        {
            ImGui::Text("Emitter Duration"); ImGui::Indent();
            bool bEmitterDurationUseRange = RequiredModule->bEmitterDurationUseRange;
            if (ImGui::Checkbox("Use Range##Duration", &bEmitterDurationUseRange))
            {
                RequiredModule->bEmitterDurationUseRange = bEmitterDurationUseRange;
            }
            ImGui::InputFloat("Value##Duration", &RequiredModule->EmitterDuration);
            if (bEmitterDurationUseRange) ImGui::InputFloat("Min##DurationRange", &RequiredModule->EmitterDurationLow); // Ensure unique ID
            ImGui::Unindent(); ImGui::Separator();

            ImGui::Text("Emitter Delay"); ImGui::Indent();
            bool bEmitterDelayUseRange = RequiredModule->bEmitterDelayUseRange;
            if (ImGui::Checkbox("Use Range##Delay", &bEmitterDelayUseRange))
            {
                RequiredModule->bEmitterDelayUseRange = bEmitterDelayUseRange;
            }
            ImGui::InputFloat("Value##Delay", &RequiredModule->EmitterDelay);
            if (bEmitterDelayUseRange) ImGui::InputFloat("Min##DelayRange", &RequiredModule->EmitterDelayLow); // Ensure unique ID
            ImGui::Unindent();
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("SubUV", TreeNodePropertyFlags))
        {
            const char* InterpMethodLabels[] = { "None", "Linear", "Linear_Blend", "Random", "Random_Blend" };
            int CurrentInterpMethod = static_cast<int>(RequiredModule->InterpolationMethod);
            if (ImGui::Combo("Interpolation Method", &CurrentInterpMethod, InterpMethodLabels, IM_ARRAYSIZE(InterpMethodLabels)))
            {
                RequiredModule->InterpolationMethod = TEnumAsByte<EParticleSubUVInterpMethod>(static_cast<uint8>(CurrentInterpMethod));
            }

            ImGui::InputInt("SubImages Horizontal", &RequiredModule->SubImages_Horizontal);
            ImGui::InputInt("SubImages Vertical", &RequiredModule->SubImages_Vertical);
            ImGui::TreePop();
        }
    }
    else if (UParticleModuleSpawn* SpawnModule = Cast<UParticleModuleSpawn>(SelectedModule))
    {
        if (ImGui::TreeNodeEx("Rate", TreeNodePropertyFlags))
        {
            bool bSpawnRateUseRange = SpawnModule->bSpawnRateUseRange;
            if (ImGui::Checkbox("Use Range##SpawnRate", &bSpawnRateUseRange))
            {
                SpawnModule->bSpawnRateUseRange = bSpawnRateUseRange;
            }
            if (!bSpawnRateUseRange)
            {
                ImGui::InputFloat("Value##SpawnRate", &SpawnModule->Rate);
            } else
            {
                ImGui::InputFloat("Max##SpawnRate", &SpawnModule->Rate);
                ImGui::InputFloat("Min##SpawnRate", &SpawnModule->RateLow); // Ensure unique ID
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNodeEx("Rate Scale", TreeNodePropertyFlags))
        {
            bool bSpawnRateScaleUseRange = SpawnModule->bSpawnRateScaleUseRange;
            if (ImGui::Checkbox("Use Range##SpawnRateScale", &bSpawnRateScaleUseRange))
            {
                SpawnModule->bSpawnRateScaleUseRange = bSpawnRateScaleUseRange;
            }
            if (!bSpawnRateScaleUseRange)
            {
                ImGui::InputFloat("Value##SpawnRateScale", &SpawnModule->RateScale);
            } else
            {
                ImGui::InputFloat("Max##SpawnRateScale", &SpawnModule->RateScale);
                ImGui::InputFloat("Min##SpawnRateScale", &SpawnModule->RateScaleLow); // Ensure unique ID
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Behavior", TreeNodePropertyFlags))
        {
            bool bApplyGlobalSpawnRateScale = SpawnModule->bApplyGlobalSpawnRateScale;
            if (ImGui::Checkbox("Apply Global Spawn Rate Scale", &bApplyGlobalSpawnRateScale))
            {
                SpawnModule->bApplyGlobalSpawnRateScale = bApplyGlobalSpawnRateScale;
            }

            bool bProcessSpawnRate = SpawnModule->bProcessSpawnRate;
            if (ImGui::Checkbox("Process Spawn Rate (Burst?)", &bProcessSpawnRate))
            {
                SpawnModule->bProcessSpawnRate = bProcessSpawnRate;
            }
            ImGui::TreePop();
        }
    }
    //else if (UParticleModuleLifetime* LifetimeModule = Cast<UParticleModuleLifetime>(SelectedModule))
    //{
    //    if (ImGui::TreeNodeEx("Lifetime", TreeNodePropertyFlags))
    //    {
    //        // LifetimeModule->Lifetime.RenderImGui("Particle Lifetime");
    //        ImGui::Text("Particle Lifetime (Constant for now):");
    //        ImGui::InputFloat("Constant", &LifetimeModule->GetLifetimeValue());
    //        ImGui::TreePop();
    //    }
    //}
    else if (UParticleModuleSize* SizeModule = Cast<UParticleModuleSize>(SelectedModule))
    {
        //if (ImGui::TreeNodeEx("Initial Size", TreeNodePropertyFlags))
        //{
        //    // SizeModule->StartSize.RenderImGui("Start Size"); // FDistributionVector
        //    ImGui::Text("Start Size (Constant Vector for now):");
        //    // Assuming FDistributionVector has Constant member that is FVector
        //    ImGui::InputFloat3("Constant XYZ", (float*)&SizeModule->StartSize.GetValue());
        //    ImGui::TreePop();
        //}
    }
    else if (UParticleModuleVelocity* VelocityModule = Cast<UParticleModuleVelocity>(SelectedModule))
    {
        //if (ImGui::TreeNodeEx("Start Velocity", TreeNodePropertyFlags))
        //{
        //    // VelocityModule->StartVelocity.RenderImGui("Start Velocity");
        //    ImGui::Text("Start Velocity (Constant Vector for now):");
        //    ImGui::InputFloat3("Constant XYZ", (float*)&VelocityModule->StartVelocity.GetValue());
        //    ImGui::TreePop();
        //}
        //if (ImGui::TreeNodeEx("Radial Velocity", TreeNodePropertyFlags))
        //{
        //    // VelocityModule->StartVelocityRadial.RenderImGui("Start Velocity Radial");
        //    ImGui::Text("Start Velocity Radial (Constant for now):");
        //    ImGui::InputFloat("Constant", &VelocityModule->StartVelocityRadial.GetValue());
        //    ImGui::TreePop();
        //}
    }
    else if (UParticleModuleColor* ColorModule = Cast<UParticleModuleColor>(SelectedModule))
    {
        //if (ImGui::TreeNodeEx("Initial Color", TreeNodePropertyFlags))
        //{
        //    // ColorModule->StartColor.RenderImGui("Start Color"); // FDistributionVector for RGB
        //    // Assuming FDistributionVector has Constant member that is FVector (used as RGB)
        //    // ImGui::ColorEdit3("Start Color", (float*)&ColorModule->StartColor.Constant);
        //    ImGui::Text("Start Color (Constant RGB for now):");
        //    ImGui::InputFloat3("RGB", (float*)&ColorModule->StartColor.GetValue());
        //    ImGui::TreePop();
        //}
        //if (ImGui::TreeNodeEx("Initial Alpha", TreeNodePropertyFlags))
        //{
        //    // ColorModule->StartAlpha.RenderImGui("Start Alpha"); // FDistributionFloat
        //    ImGui::Text("Start Alpha (Constant for now):");
        //    float Color = ColorModule->StartAlpha.GetValue();
        //    ImGui::InputFloat("Constant", (float*)&ColorModule->StartAlpha.GetValue());
        //    ImGui::TreePop();
        //}
    }
    else if (UParticleModuleSubUV* SubUVModule = Cast<UParticleModuleSubUV>(SelectedModule))
    {
        //if (ImGui::TreeNodeEx("SubUV Animation", TreeNodePropertyFlags))
        //{
        //    // SubUVModule->SubImageIndex.RenderImGui("SubImage Index");
        //    ImGui::Text("SubImage Index (Constant for now):");
        //    ImGui::InputFloat("Constant", &SubUVModule->SubImageIndex.GetValue());
        //    // Add other SubUV properties (InterpolationMethod, etc., if not in Required)
        //    ImGui::TreePop();
        //}
    }
    else if (UParticleModuleTypeDataMesh* TypeDataMeshModule = Cast<UParticleModuleTypeDataMesh>(SelectedModule))
    {
        if (ImGui::TreeNodeEx("Mesh Properties", TreeNodePropertyFlags))
        {
            // 
            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::TextWrapped("Properties editor for this module type ('%s') is not yet implemented.", (*ModuleClassName));
    }

    ImGui::PopStyleColor();
}

void FParticleEditorPanel::CalculatePanelSize(RECT InRect)
{
    WinWidth = static_cast<float>(InRect.right - InRect.left);
    float WinHeight = static_cast<float>(InRect.bottom - InRect.top);

    if (WinWidth <= 0) WinWidth = 1280;
    if (WinHeight <= 0) WinHeight = 720;

    MenuBarHeight = 32.0f;
    ToolBarHeight = 32.0f;
    float TopBarsTotalHeight = MenuBarHeight + ToolBarHeight;

    UsableScreenYOffset = TopBarsTotalHeight;
    UsableScreenHeight = WinHeight - TopBarsTotalHeight;
    if (UsableScreenHeight < 0.0f) UsableScreenHeight = 0.0f;

    float EmitterPanelWidthRatio = 0.75f;
    EmitterPanelWidth = WinWidth * EmitterPanelWidthRatio;
    if (EmitterPanelWidth < 0.0f) EmitterPanelWidth = 0.0f;
    EmitterPanelXPos = WinWidth - EmitterPanelWidth;
    if (EmitterPanelXPos < 0.0f) EmitterPanelXPos = 0.0f;

    LeftAreaWidth = WinWidth - EmitterPanelWidth;
    if (LeftAreaWidth < 0.0f) LeftAreaWidth = 0.0f;

    float DetailPanelHeightRatio = 0.5f;
    DetailPanelHeight = UsableScreenHeight * DetailPanelHeightRatio;
    if (DetailPanelHeight < 0.0f) DetailPanelHeight = 0.0f;
    DetailPanelYPos = UsableScreenYOffset + (UsableScreenHeight - DetailPanelHeight);

    ViewportPanelHeight = UsableScreenHeight - DetailPanelHeight;
    if (ViewportPanelHeight < 0.0f) ViewportPanelHeight = 0.0f;

}
