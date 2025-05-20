#include "ParticleEditorPanel.h"
//#include "Engine/EditorEngine.h"

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

FParticleEditorPanel::FParticleEditorPanel()
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

void FParticleEditorPanel::Render()
{
    // 좌측 상단에 버튼 배치
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(240, 50), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
    ImGui::Begin("ParticleEditorModeBar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse
    );
    /*if (ImGui::Button("Exit Particle Editor Mode", ImVec2(220, 30)))
    {
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EditorEngine->EndParticlePreviewMode();
        }
    }*/
    ImGui::End();
    ImGui::PopStyleColor();

    // 윈도우 크기 얻기
    HWND Hwnd = ::GetActiveWindow();
    RECT ClientRect;
    GetClientRect(Hwnd, &ClientRect);
    int WinWidth = ClientRect.right - ClientRect.left;
    int WinHeight = ClientRect.bottom - ClientRect.top;

    // 패널 비율 설정
    float EmitterPanelWidth = WinWidth * 0.3f;   // 우측 30%
    float DetailPanelHeight = WinHeight * 0.3f;  // 하단 30%

    // 1. 우측 Emitters 패널
    ImVec2 EmitterPanelPos(WinWidth - EmitterPanelWidth, 0);
    ImVec2 EmitterPanelSize(EmitterPanelWidth, WinHeight);

    RenderEmitterPanel(EmitterPanelPos, EmitterPanelSize);


    // 2. 하단 Properties(Detail) 패널
    ImVec2 DetailPanelPos(0, WinHeight - DetailPanelHeight);
    ImVec2 DetailPanelSize(WinWidth - EmitterPanelWidth, DetailPanelHeight);

    RenderDetailPanel(DetailPanelPos, DetailPanelSize);

}

void FParticleEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
}

void FParticleEditorPanel::RenderMenuBar()
{

}

void FParticleEditorPanel::RenderToolBar(const ImVec2& InSize)
{

}

void FParticleEditorPanel::RenderViewportPanel(const ImVec2& InSize)
{

}

void FParticleEditorPanel::RenderDetailPanel(const ImVec2& InPos, const ImVec2& InSize)
{
    ImGui::SetNextWindowPos(InPos);
    ImGui::SetNextWindowSize(InSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
    ImGui::Begin("Properties", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize
    );
    
    RenderDetailInfos();

    ImGui::End();
    ImGui::PopStyleColor();
}

void FParticleEditorPanel::RenderEmitterPanel(const ImVec2& InPos, const ImVec2& InSize)
{
    ImGui::SetNextWindowPos(InPos);
    ImGui::SetNextWindowSize(InSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
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

void FParticleEditorPanel::RenderDetailInfos()
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
