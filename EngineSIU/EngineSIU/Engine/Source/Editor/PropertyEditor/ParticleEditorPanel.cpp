#include "ParticleEditorPanel.h"

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

ParticleEditorPanel::ParticleEditorPanel()
{
    // 초기화 필요시 작성

    // TEST
    UParticleEmitter* Emmitter = FObjectFactory::ConstructObject<UParticleEmitter>(nullptr);
    Emmitter->LODLevels.Add(FObjectFactory::ConstructObject<UParticleLODLevel>(Emmitter));
    Emmitter->LODLevels[0]->Modules.Add(FObjectFactory::ConstructObject<UParticleModuleRequired>(Emmitter->LODLevels[0]));
    Emmitter->LODLevels[0]->Modules.Add(FObjectFactory::ConstructObject<UParticleModuleSpawn>(Emmitter->LODLevels[0]));
    Emmitter->LODLevels[0]->Modules.Add(FObjectFactory::ConstructObject<UParticleModuleLifetime>(Emmitter->LODLevels[0]));
    Emmitter->LODLevels[0]->Modules.Add(FObjectFactory::ConstructObject<UParticleModuleSize>(Emmitter->LODLevels[0]));
    Emmitter->LODLevels[0]->Modules.Add(FObjectFactory::ConstructObject<UParticleModuleVelocity>(Emmitter->LODLevels[0]));
    Emmitter->LODLevels[0]->Modules.Add(FObjectFactory::ConstructObject<UParticleModuleColor>(Emmitter->LODLevels[0]));

    TargetParticleSystem = FObjectFactory::ConstructObject<UParticleSystem>(nullptr);
    TargetParticleSystem->Emitters.Add(Emmitter);
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
    RenderDetailInfos();
    ImGui::EndChild();
}

void ParticleEditorPanel::RenderEmitterPanel(const ImVec2& InSize)
{
    ImGui::BeginChild("EmitterPanel", InSize, true);
    
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::TextColored(ImVec4(1, 1, 1, 1), " Emitters");
    ImGui::Separator();
    RenderEmitterInfos();
    ImGui::EndChild();
}

void ParticleEditorPanel::RenderEmitterInfos()
{
    if (!TargetParticleSystem)
        return;
    for (int i = 0; i < TargetParticleSystem->Emitters.Num(); ++i)
    {
        UParticleEmitter* Emitter = TargetParticleSystem->Emitters[i];
        ImGui::BeginChild("EmitterInfo##", ImVec2(200, 0));

        TArray<UParticleModule*>& Modules = Emitter->LODLevels[0]->Modules;
        for (int j = 0; j < Modules.Num(); ++j)
        {
            UParticleModule* Module = Modules[j];
            bool bSelected = (Module == SelectedModule);
            ImGui::Selectable(GetData(Module->GetName() + "##"), &bSelected);
            if (bSelected)
            {
                SelectedModule = Module;
            }       
        }
        ImGui::EndChild();
    }
}

void ParticleEditorPanel::RenderDetailInfos()
{
    if (SelectedModule == nullptr)
    {
        return;
    } else if (UParticleModuleRequired* RequiredModule = Cast<UParticleModuleRequired>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("Required");
        if (ImGui::TreeNodeEx("Emitter", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) 
        {
            FVector Origin = RequiredModule->EmitterOrigin;
            FImGuiWidget::DrawVec3Control("Origin", Origin, 0, 85);
            RequiredModule->EmitterOrigin = Origin;
            ImGui::Spacing();
    
            FRotator Rotation = RequiredModule->EmitterRotation;
            FImGuiWidget::DrawRot3Control("Rotation", Rotation, 0, 85);
            RequiredModule->EmitterRotation = Rotation;
            ImGui::Spacing();

            bool bUseLocalSpace = RequiredModule->bUseLocalSpace;
            ImGui::Checkbox("Use Local Space", &bUseLocalSpace);
            RequiredModule->bUseLocalSpace = bUseLocalSpace;

            bool bKillOnDeactive = RequiredModule->bKillOnDeactivate;
            ImGui::Checkbox("Kill On Deactive", &bKillOnDeactive);
            RequiredModule->bKillOnDeactivate = bKillOnDeactive;
            
            bool bKillOnCompleted = RequiredModule->bKillOnCompleted;
            ImGui::Checkbox("Kill On Completed", &bKillOnCompleted);
            RequiredModule->bKillOnCompleted = bKillOnCompleted;

            const char* Label[5] = {"None", "ViewProjDepth", "DistanceToView", "Age_OldestFirst", "Age_NewestFirst"};
            int SortMode = RequiredModule->SortMode;
            ImGui::Combo("Sorting Mode", &SortMode, Label, 5);
            RequiredModule->SortMode = TEnumAsByte<EParticleSortMode>(SortMode);
            
            ImGui::TreePop();
        }
    
        if (ImGui::TreeNodeEx("Elapsed Time", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool bEmitterDurationUseRange = RequiredModule->bEmitterDurationUseRange;
            ImGui::Checkbox("Use Emitter Duration with Range", &bEmitterDurationUseRange);
            RequiredModule->bEmitterDurationUseRange = bEmitterDurationUseRange;

            ImGui::InputFloat("Duration##", &RequiredModule->EmitterDuration);
            ImGui::InputFloat("Duration Low##", &RequiredModule->EmitterDurationLow);

            ImGui::TreePop();
        }
        
        if (ImGui::TreeNodeEx("Delay Time", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool bEmitterDelayUseRange = RequiredModule->bEmitterDelayUseRange;
            ImGui::Checkbox("Use Emitter Delay with Range", &bEmitterDelayUseRange);
            RequiredModule->bEmitterDelayUseRange = bEmitterDelayUseRange;

            ImGui::InputFloat("Delay##", &RequiredModule->EmitterDelay);
            ImGui::InputFloat("Delay Low##", &RequiredModule->EmitterDelayLow);

            ImGui::TreePop();
        }
        
        if (ImGui::TreeNodeEx("Sub UV", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* Label[5] = {"None", "Linear", "Linear Blend", "Random", "Random_blend"};
            int selected = RequiredModule->InterpolationMethod;
            ImGui::Combo("Interpolation Method", &selected, Label, 5);
            RequiredModule->InterpolationMethod = TEnumAsByte<EParticleSubUVInterpMethod>(selected);

            // bool bUseScaleUV;

            ImGui::InputInt("Horizontal SubImage", &RequiredModule->SubImages_Horizontal);
            ImGui::InputInt("Vertical SubImage", &RequiredModule->SubImages_Vertical);
            
            ImGui::TreePop();
        }
        
        ImGui::PopStyleColor();
        
    } else if (UParticleModuleSpawn* SpawnModule = Cast<UParticleModuleSpawn>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("Spawn");
        if (ImGui::TreeNodeEx("Spawn", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::TreeNodeEx("Rate", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                const char* Label[3] = {"None", "FloatConstant", "FloatUniform"};
                FDistributionFloat& dist = SpawnModule->Rate;
                EDistributionType DistributionType = SpawnModule->Rate.GetType();
                int DistributionTypeIndex = static_cast<int>(DistributionType);
                ImGui::Combo("Distribution", &DistributionTypeIndex, Label, 3);
                if (DistributionTypeIndex != static_cast<int>(DistributionType))
                {
                    switch (static_cast<EDistributionType>(DistributionTypeIndex))
                    {
                    case EDistributionType::FloatConstant:
                        SpawnModule->Rate = FDistributionFloatConstant();
                        break;
                    case EDistributionType::FloatUniform:
                        SpawnModule->Rate = FDistributionFloatUniform();
                        break;
                    default:
                        assert(0);
                    }
                }
                SpawnModule->Rate.RenderProperty();
                ImGui::TreePop();
            }
    
            if (ImGui::TreeNodeEx("Rate Scale", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                const char* Label[3] = {"None", "FloatConstant", "FloatUniform"};
            
                EDistributionType DistributionType = SpawnModule->RateScale.GetType();
                int DistributionTypeIndex = static_cast<int>(DistributionType);
                ImGui::Combo("Distribution", &DistributionTypeIndex, Label, 3);
                if (DistributionTypeIndex != static_cast<int>(DistributionType))
                {
                    switch (static_cast<EDistributionType>(DistributionTypeIndex))
                    {
                    case EDistributionType::FloatConstant:
                        SpawnModule->RateScale = FDistributionFloatConstant();
                        break;
                    case EDistributionType::FloatUniform:
                        SpawnModule->RateScale = FDistributionFloatUniform();
                        break;
                    default:
                        assert(0);
                    }
                }
                SpawnModule->RateScale.RenderProperty();
                ImGui::TreePop();
            }

            bool bApplyGlobalSpawnRateScale = SpawnModule->bApplyGlobalSpawnRateScale;
            ImGui::Checkbox("Apply Global Spawn Rate", &bApplyGlobalSpawnRateScale);
            SpawnModule->bApplyGlobalSpawnRateScale = bApplyGlobalSpawnRateScale;

            bool bProcessSpawnRate = SpawnModule->bProcessSpawnRate;
            ImGui::Checkbox("Process Spawn Rate", &bProcessSpawnRate);
            SpawnModule->bProcessSpawnRate = bProcessSpawnRate;
            
            ImGui::TreePop();
        }
        ImGui::PopStyleColor(); 
    } else if (UParticleModuleLocation* LocationModule = Cast<UParticleModuleLocation>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("Location");
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
             
            ImGui::TreePop();
        }
    
        ImGui::PopStyleColor(); 
    } else if (UParticleModuleVelocity* VelocityModule = Cast<UParticleModuleVelocity>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("Velocity");
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
             
            ImGui::TreePop();
        }
    
        ImGui::PopStyleColor(); 
    } else if (UParticleModuleSize* SizeModule = Cast<UParticleModuleSize>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("Size");
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
             
            ImGui::TreePop();
        }
    
        ImGui::PopStyleColor(); 
    } else if (UParticleModuleLifetime* LifetimeModule = Cast<UParticleModuleLifetime>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("Lifetime");
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
             
            ImGui::TreePop();
        }
    
        ImGui::PopStyleColor(); 
    } else if (UParticleModuleColor* ColorModule = Cast<UParticleModuleColor>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("Color");
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
             
            ImGui::TreePop();
        }
    
        ImGui::PopStyleColor(); 
    } else if (UParticleModuleSubUV* SubUVModule = Cast<UParticleModuleSubUV>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("SubUV");
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
             
            ImGui::TreePop();
        }
    
        ImGui::PopStyleColor(); 
    } else if (UParticleModuleTypeDataMesh* TypeDataMeshModule = Cast<UParticleModuleTypeDataMesh>(SelectedModule))
    {
        ImGui::SetItemDefaultFocus();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        ImGui::Text("TypeDataMesh");
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
             
            ImGui::TreePop();
        }
    
        ImGui::PopStyleColor(); 
    }
}
