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

#include "Asset/StaticMeshAsset.h"
#include "Components/Mesh/StaticMeshRenderData.h"
#include "Engine/AssetManager.h"

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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.01f, 1.f));

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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.01f, 1.0f));
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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.01f, 1.f));
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
                if (Emitter->LODLevels.Num() > 0)
                {
                    UParticleLODLevel* LODLevel = Emitter->GetLODLevel(0);
                    {
                        bool bSelected = (LODLevel->RequiredModule == SelectedModule);
                        if (ImGui::Selectable("Required Module", bSelected, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAllColumns))
                        {
                            SelectedModule = LODLevel->RequiredModule;
                        }
                    }
                    {
                        bool bSelected = (LODLevel->SpawnModule == SelectedModule);
                        if (ImGui::Selectable("Spawn Module", bSelected, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAllColumns))
                        {
                            SelectedModule = LODLevel->SpawnModule;
                        }
                    }

                    {
                        bool bSelected = (LODLevel->TypeDataModule == SelectedModule);
                        if (ImGui::Selectable("Type Data Module", bSelected, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAllColumns))
                        {
                            SelectedModule = LODLevel->TypeDataModule;
                        }
                    }
                    
                    for (UParticleModule* Module : LODLevel->Modules)
                    {
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

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            if (ImGui::Selectable(GetData(RequiredModule->Material->GetName()), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    // IsOpen = true;
                }
            }

            // if (ImGui::Button("    +    "))
            // {
                // IsCreateMaterial = true;
            // }

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
        
        // if (IsOpen != -1)
        // {
        //     RenderMaterialView(RequiredModule->Material);
        // }
        // if (IsCreateMaterial)
        // {
        //     RenderCreateMaterialView();
        // }
        
        
        if (ImGui::TreeNodeEx("Transform", TreeNodePropertyFlags))
        {
            FImGuiWidget::DrawVec3Control("Emitter Origin", RequiredModule->EmitterOrigin, 0.0f, 100.0f);
            ImGui::Spacing();
            FImGuiWidget::DrawRot3Control("Emitter Rotation", RequiredModule->EmitterRotation, 0.0f, 100.0f);
            
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

            bool bIsTranslucent = RequiredModule->bIsTranslucent;
            if (ImGui::Checkbox("Is Translucent", &bIsTranslucent))
            {
                RequiredModule->bIsTranslucent = bIsTranslucent;
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
        ImGui::DragFloat("Rate", &SpawnModule->Rate.Constant);
        ImGui::DragFloat("RateScale", &SpawnModule->RateScale.Constant);

        bool bApplyGlobalSpawnRateScale = SpawnModule->bApplyGlobalSpawnRateScale;
        if (ImGui::Checkbox("Apply Global Spawn Rate Scale", &bApplyGlobalSpawnRateScale))
        {
            SpawnModule->bApplyGlobalSpawnRateScale = bApplyGlobalSpawnRateScale;
        }

        bool bProcessSpawnRate = SpawnModule->bProcessSpawnRate;
        if (ImGui::Checkbox("Process Spawn Rate", &bProcessSpawnRate))
        {
            SpawnModule->bProcessSpawnRate = bProcessSpawnRate;
        }
    }
    else if (UParticleModuleLifetime* LifetimeModule = Cast<UParticleModuleLifetime>(SelectedModule))
    {
        if (ImGui::TreeNodeEx("Lifetime", TreeNodePropertyFlags))
        {
            // ImGui::Text("Particle Lifetime (Constant for now):");
            ImGui::DragFloat("Max", &LifetimeModule->Lifetime.Max);
            ImGui::DragFloat("Min", &LifetimeModule->Lifetime.Min);
            ImGui::TreePop();
        }
    }
    else if (UParticleModuleSize* SizeModule = Cast<UParticleModuleSize>(SelectedModule))
    {
        if (ImGui::TreeNodeEx("Start Size", TreeNodePropertyFlags))
        {
            // ImGui::Text("Start Size (Constant Vector for now):");
            FImGuiWidget::DrawVec3Control("Max", SizeModule->StartSize.Max, 0, 85);
            FImGuiWidget::DrawVec3Control("Min", SizeModule->StartSize.Min, 0, 85);
            ImGui::TreePop();
        }
    }
    else if (UParticleModuleVelocity* VelocityModule = Cast<UParticleModuleVelocity>(SelectedModule))
    {
        if (ImGui::TreeNodeEx("Start Velocity", TreeNodePropertyFlags))
        {
            // ImGui::Text("Start Velocity (Constant Vector for now):");
            FImGuiWidget::DrawVec3Control("Max", VelocityModule->StartVelocity.Max, 0, 85);
            FImGuiWidget::DrawVec3Control("Min", VelocityModule->StartVelocity.Min, 0, 85);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Radial Velocity", TreeNodePropertyFlags))
        {
            // ImGui::Text("Start Velocity Radial (Constant for now):");
            FImGuiWidget::DrawVec3Control("Max", VelocityModule->StartVelocityRadial.Max, 0, 85);
            FImGuiWidget::DrawVec3Control("Min", VelocityModule->StartVelocityRadial.Min, 0, 85);
            ImGui::TreePop();
        }
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
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            if (ImGui::TreeNodeEx("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
            {
                ImGui::Text("StaticMesh");
                ImGui::SameLine();

                FString PreviewName = FString("None");
                if (UStaticMesh* StaticMesh = TypeDataMeshModule->Mesh)
                {
                    if (FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData())
                    {
                        PreviewName = RenderData->DisplayName;
                    }
                }
        
                const TMap<FName, FAssetInfo> Assets = UAssetManager::Get().GetAssetRegistry();

                if (ImGui::BeginCombo("##StaticMesh", GetData(PreviewName), ImGuiComboFlags_None))
                {
                    for (const auto& Asset : Assets)
                    {
                        if (ImGui::Selectable(GetData(Asset.Value.AssetName.ToString()), false))
                        {
                            FString MeshName = Asset.Value.PackagePath.ToString() + "/" + Asset.Value.AssetName.ToString();
                            UStaticMesh* StaticMesh = FResourceManager::GetStaticMesh(MeshName.ToWideString());
                            if (StaticMesh)
                            {
                                TypeDataMeshModule->Mesh = StaticMesh;
                            }
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::TreePop();
            }
            ImGui::PopStyleColor();

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

void FParticleEditorPanel::RenderMaterialView(UMaterial* Material)
{
    // ImGui::SetNextWindowSize(ImVec2(380, 400), ImGuiCond_Once);
    // ImGui::Begin("Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);
    //
    // static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;
    //
    // const FVector MatDiffuseColor = Material->GetMaterialInfo().DiffuseColor;
    // const FVector MatSpecularColor = Material->GetMaterialInfo().SpecularColor;
    // const FVector MatAmbientColor = Material->GetMaterialInfo().AmbientColor;
    // const FVector MatEmissiveColor = Material->GetMaterialInfo().EmissiveColor;
    //
    // const float DiffuseR = MatDiffuseColor.X;
    // const float DiffuseG = MatDiffuseColor.Y;
    // const float DiffuseB = MatDiffuseColor.Z;
    // constexpr float DiffuseA = 1.0f;
    // float DiffuseColorPick[4] = { DiffuseR, DiffuseG, DiffuseB, DiffuseA };
    //
    // ImGui::Text("Material Name |");
    // ImGui::SameLine();
    // ImGui::Text(*Material->GetMaterialInfo().MaterialName);
    // ImGui::Separator();
    //
    // ImGui::Text("  Diffuse Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Diffuse##Color", reinterpret_cast<float*>(&DiffuseColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
    //     Material->SetDiffuse(NewColor);
    // }
    //
    // const float SpecularR = MatSpecularColor.X;
    // const float SpecularG = MatSpecularColor.Y;
    // const float SpecularB = MatSpecularColor.Z;
    // constexpr float SpecularA = 1.0f;
    // float SpecularColorPick[4] = { SpecularR, SpecularG, SpecularB, SpecularA };
    //
    // ImGui::Text("Specular Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Specular##Color", reinterpret_cast<float*>(&SpecularColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
    //     Material->SetSpecular(NewColor);
    // }
    //
    // const float AmbientR = MatAmbientColor.X;
    // const float AmbientG = MatAmbientColor.Y;
    // const float AmbientB = MatAmbientColor.Z;
    // constexpr float AmbientA = 1.0f;
    // float AmbientColorPick[4] = { AmbientR, AmbientG, AmbientB, AmbientA };
    //
    // ImGui::Text("Ambient Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Ambient##Color", reinterpret_cast<float*>(&AmbientColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
    //     Material->SetAmbient(NewColor);
    // }
    //
    // const float EmissiveR = MatEmissiveColor.X;
    // const float EmissiveG = MatEmissiveColor.Y;
    // const float EmissiveB = MatEmissiveColor.Z;
    // constexpr float EmissiveA = 1.0f;
    // float EmissiveColorPick[4] = { EmissiveR, EmissiveG, EmissiveB, EmissiveA };
    //
    // ImGui::Text("Emissive Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Emissive##Color", reinterpret_cast<float*>(&EmissiveColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
    //     Material->SetEmissive(NewColor);
    // }
    //
    // ImGui::Spacing();
    // ImGui::Separator();
    //
    // ImGui::Text("Choose Material");
    // ImGui::Spacing();
    //
    // ImGui::Text("Material Slot Name |");
    // ImGui::SameLine();
    // ImGui::Text(GetData(Material->GetName()));
    //
    // ImGui::Text("Override Material |");
    // ImGui::SameLine();
    // ImGui::SetNextItemWidth(160);
    // // 메테리얼 이름 목록을 const char* 배열로 변환
    // std::vector<const char*> MaterialChars;
    // for (const auto& Material : FResourceManager::GetMaterials()) {
    //     MaterialChars.push_back(*Material.Value->GetMaterialInfo().MaterialName);
    // }
    //
    // //// 드롭다운 표시 (currentMaterialIndex가 범위를 벗어나지 않도록 확인)
    // //if (currentMaterialIndex >= FManagerGetMaterialNum())
    // //    currentMaterialIndex = 0;
    //
    // if (ImGui::Combo("##MaterialDropdown", &CurMaterialIndex, MaterialChars.data(), FResourceManager::GetMaterialNum())) {
    //     UMaterial* Material = FResourceManager::GetMaterial(MaterialChars[CurMaterialIndex]);
    //     SelectedStaticMeshComp->SetMaterial(SelectedMaterialIndex, Material);
    // }
    //
    // if (ImGui::Button("Close"))
    // {
    //     SelectedMaterialIndex = -1;
    //     SelectedStaticMeshComp = nullptr;
    // }
    //
    // ImGui::End();
}

void FParticleEditorPanel::RenderCreateMaterialView()
{
    // ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Once);
    // ImGui::Begin("Create Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);
    //
    // static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;
    //
    // ImGui::Text("New Name");
    // ImGui::SameLine();
    // static char MaterialName[256] = "New Material";
    // // 기본 텍스트 입력 필드
    // ImGui::SetNextItemWidth(128);
    // if (ImGui::InputText("##NewName", MaterialName, IM_ARRAYSIZE(MaterialName))) {
    //     tempMaterialInfo.MaterialName = MaterialName;
    // }
    //
    // const FVector MatDiffuseColor = tempMaterialInfo.DiffuseColor;
    // const FVector MatSpecularColor = tempMaterialInfo.SpecularColor;
    // const FVector MatAmbientColor = tempMaterialInfo.AmbientColor;
    // const FVector MatEmissiveColor = tempMaterialInfo.EmissiveColor;
    //
    // const float DiffuseR = MatDiffuseColor.X;
    // const float DiffuseG = MatDiffuseColor.Y;
    // const float DiffuseB = MatDiffuseColor.Z;
    // constexpr float DiffuseA = 1.0f;
    // float DiffuseColorPick[4] = { DiffuseR, DiffuseG, DiffuseB, DiffuseA };
    //
    // ImGui::Text("Set Property");
    // ImGui::Indent();
    //
    // ImGui::Text("  Diffuse Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Diffuse##Color", reinterpret_cast<float*>(&DiffuseColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
    //     tempMaterialInfo.DiffuseColor = NewColor;
    // }
    //
    // const float SpecularR = MatSpecularColor.X;
    // const float SpecularG = MatSpecularColor.Y;
    // const float SpecularB = MatSpecularColor.Z;
    // constexpr float SpecularA = 1.0f;
    // float SpecularColorPick[4] = { SpecularR, SpecularG, SpecularB, SpecularA };
    //
    // ImGui::Text("Specular Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Specular##Color", reinterpret_cast<float*>(&SpecularColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
    //     tempMaterialInfo.SpecularColor = NewColor;
    // }
    //
    // const float AmbientR = MatAmbientColor.X;
    // const float AmbientG = MatAmbientColor.Y;
    // const float AmbientB = MatAmbientColor.Z;
    // constexpr float AmbientA = 1.0f;
    // float AmbientColorPick[4] = { AmbientR, AmbientG, AmbientB, AmbientA };
    //
    // ImGui::Text("Ambient Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Ambient##Color", reinterpret_cast<float*>(&AmbientColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
    //     tempMaterialInfo.AmbientColor = NewColor;
    // }
    //
    // const float EmissiveR = MatEmissiveColor.X;
    // const float EmissiveG = MatEmissiveColor.Y;
    // const float EmissiveB = MatEmissiveColor.Z;
    // constexpr float EmissiveA = 1.0f;
    // float EmissiveColorPick[4] = { EmissiveR, EmissiveG, EmissiveB, EmissiveA };
    //
    // ImGui::Text("Emissive Color");
    // ImGui::SameLine();
    // if (ImGui::ColorEdit4("Emissive##Color", reinterpret_cast<float*>(&EmissiveColorPick), BaseFlag))
    // {
    //     const FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
    //     tempMaterialInfo.EmissiveColor = NewColor;
    // }
    // ImGui::Unindent();
    //
    // ImGui::NewLine();
    // if (ImGui::Button("Create Material")) {
    //     FResourceManager::CreateMaterial(tempMaterialInfo);
    // }
    //
    // ImGui::NewLine();
    // if (ImGui::Button("Close"))
    // {
    //     IsCreateMaterial = false;
    // }
    //
    // ImGui::End();
}
