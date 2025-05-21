#include "EditorEngine.h"

#include "World/World.h"
#include "Level.h"
#include "Actors/Cube.h"
#include "Actors/LightActors/DirectionalLightActor.h"
#include "GameFramework/Actor.h"
#include "Classes/Engine/AssetManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "UObject/UObjectIterator.h"

#include "Engine/Source/Runtime/Launch/EngineLoop.h"
#include "Engine/Source/Editor/LevelEditor/SLevelEditor.h"
#include "Engine/Source/Editor/UnrealEd/EditorViewportClient.h"

#include "Components/SkeletalMesh/SkeletalMesh.h"
#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "DataPreviewController.h"

#include "Actors/Character/Pawn.h"
#include "Animation/UAnimInstance.h"
#include "UObject/Casts.h"

#include "Particles/ParticleSystemComponent.h"
#include "Engine/ParticlePreviewController.h"
#include "Actors/PrimitiveActors/AParticleActor.h"

#include "UnrealEd/UnrealEd.h"
#include "PropertyEditor/ParticleEditorPanel.h"

extern FEngineLoop GEngineLoop;

namespace PrivateEditorSelection
{
    static AActor* GActorSelected = nullptr;
    static AActor* GActorHovered = nullptr;

    static USceneComponent* GComponentSelected = nullptr;
    static USceneComponent* GComponentHovered = nullptr;
}

void UEditorEngine::Init()
{
    Super::Init();

    // Initialize the engine
    GEngine = this;

    FWorldContext& EditorWorldContext = CreateNewWorldContext(EWorldType::Editor);

    EditorWorld = UWorld::CreateWorld(this, EWorldType::Editor, FString("EditorWorld"));

    EditorWorldContext.SetCurrentWorld(EditorWorld);
    ActiveWorld = EditorWorld;

    EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>(this);

    if (AssetManager == nullptr)
    {
        AssetManager = FObjectFactory::ConstructObject<UAssetManager>(this);
        assert(AssetManager);
        AssetManager->InitAssetManager();
    }
    LoadLevel("Saved/AutoSaves.scene");  
}

void UEditorEngine::Release()
{
    SaveLevel("Saved/AutoSaves.scene");
}

void UEditorEngine::Tick(float DeltaTime)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            if (UWorld* World = WorldContext->World())
            {
                // TODO: World에서 EditorPlayer 제거 후 Tick 호출 제거 필요.
                World->Tick(DeltaTime);
                EditorPlayer->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                TArray CachedActors = Level->Actors;
                if (Level)
                {
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor && Actor->IsActorTickInEditor())
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
        else if (WorldContext->WorldType == EWorldType::PIE)
        {
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                TArray CachedActors = Level->Actors;
                if (Level)
                {
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
        //else if (WorldContext->WorldType == EWorldType::SkeletalMeshEditor)
        //{
        //    if (UWorld* World = WorldContext->World())
        //    {
        //        World->Tick(DeltaTime);
        //        EditorPlayer->Tick(DeltaTime);
        //        ULevel* Level = World->GetActiveLevel();
        //        TArray CachedActors = Level->Actors;
        //        if (Level)
        //        {
        //            for (AActor* Actor : CachedActors)
        //            {
        //                if (Actor)
        //                {
        //                    Actor->Tick(DeltaTime);
        //                }
        //            }
        //        }
        //    }
        //}
        else if (WorldContext->WorldType == EWorldType::EditorPreview)
        {
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                EditorPlayer->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                TArray CachedActors = Level->Actors;
                if (Level)
                {
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
        else if (WorldContext->WorldType == EWorldType::ParticlePreview)
        {
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                EditorPlayer->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                TArray CachedActors = Level->Actors;
                if (Level)
                {
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
    }
}

void UEditorEngine::StartPIE()
{
    if (PIEWorld)
    {
        UE_LOG(LogLevel::Warning, TEXT("PIEWorld already exists!"));
        return;
    }
    this->ClearActorSelection(); // Editor World 기준 Select Actor 해제 
    
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnPIEModeStart();

    FWorldContext& PIEWorldContext = CreateNewWorldContext(EWorldType::PIE);

    PIEWorld = Cast<UWorld>(EditorWorld->Duplicate(this));
    PIEWorld->WorldType = EWorldType::PIE;

    PIEWorldContext.SetCurrentWorld(PIEWorld);
    ActiveWorld = PIEWorld;
    
    BindEssentialObjects();
    
    PIEWorld->BeginPlay();
    // 여기서 Actor들의 BeginPlay를 해줄지 안에서 해줄 지 고민.
    // WorldList.Add(GetWorldContextFromWorld(PIEWorld));
}

void UEditorEngine::BindEssentialObjects()
{
    // TODO: 플레이어 컨트롤러가 먼저 만들어져야 함.
    //실수로 안만들면 넣어주기
    if (ActiveWorld->GetMainPlayer() == nullptr)
    {
        APlayer* TempPlayer = ActiveWorld->SpawnActor<APlayer>();
        TempPlayer->SetActorLabel(TEXT("OBJ_PLAYER"));
        TempPlayer->SetActorTickInEditor(false);
        ActiveWorld->SetMainPlayer(TempPlayer);
    }
    
    //마찬가지
    for (const auto iter: TObjectRange<APlayer>())
    {
        if (iter->GetWorld() == ActiveWorld)
        {
            ActiveWorld->SetMainPlayer(iter);
            break;
        }
    }
    
    //무조건 PIE들어갈때 만들어주기
    APlayerController* PlayerController = ActiveWorld->SpawnActor<APlayerController>();
    PlayerController->SetActorLabel(TEXT("OBJ_PLAYER_CONTROLLER"));
    PlayerController->SetActorTickInEditor(false);
    ActiveWorld->SetPlayerController(PlayerController);

    
    ActiveWorld->GetPlayerController()->Possess(ActiveWorld->GetMainPlayer());
}

void UEditorEngine::EndPIE()
{
    if (PIEWorld)
    {
        this->ClearActorSelection(); // PIE World 기준 Select Actor 해제 
        //WorldList.Remove(*GetWorldContextFromWorld(PIEWorld.get()));
        WorldList.Remove(GetWorldContextFromWorld(PIEWorld));
        PIEWorld->Release();
        GUObjectArray.MarkRemoveObject(PIEWorld);
        PIEWorld = nullptr;

        // TODO: PIE에서 EditorWorld로 돌아올 때, 기존 선택된 Picking이 유지되어야 함. 현재는 에러를 막기위해 임시조치.
        DeselectActor(GetSelectedActor());
        DeselectComponent(GetSelectedComponent());
    }

    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnPIEModeEnd();
    // 다시 EditorWorld로 돌아옴.
    ActiveWorld = EditorWorld;
}

void UEditorEngine::StartEditorPreviewMode()
{
    if (PreviewWorld)
        return;

    FWorldContext& SkeletalMeshEditorWorldContext = CreateNewWorldContext(EWorldType::EditorPreview);

    PreviewWorld = UWorld::CreateWorld(this, EWorldType::EditorPreview, TEXT("Preview Mode"));

    SkeletalMeshEditorWorldContext.SetCurrentWorld(PreviewWorld);

    ActiveWorld = PreviewWorld;
}

void UEditorEngine::EndEditorPreviewMode()
{
    if (!PreviewWorld)
        return;

    // Context 제거
    if (FWorldContext* ctx = GetEditorPreviewWorldContext())
    {
        WorldList.Remove(ctx);
    }

    // World 정리
    PreviewWorld->Release();
    GUObjectArray.MarkRemoveObject(PreviewWorld);
    PreviewWorld = nullptr;

    // 원래 EditorWorld 로 복귀
    ActiveWorld = EditorWorld;
}

void UEditorEngine::StartSkeletalMeshEditMode(USkeletalMesh* InMesh)
{
    StartEditorPreviewMode();
    FEditorViewportClient* ViewPort = GEngineLoop.GetLevelEditor()->GetViewports()->get();
    DataPreviewController = std::make_shared<FDataPreviewController>(ActiveWorld, ViewPort);
    DataPreviewController->Initialize(InMesh);

    // TODO : Initialize에서 InMesh를 받아서 처리하도록 변경
    AActor* PreviewActor = ActiveWorld->SpawnActor<AActor>();
    PreviewActor->SetActorLabel(FString(InMesh->GetObjectName()));

    USkeletalMeshComponent* SkelMeshComp = PreviewActor->AddComponent<USkeletalMeshComponent>();
    SkelMeshComp->SetSkeletalMesh(DataPreviewController->OriginalMesh);
}

void UEditorEngine::StartAnimaitonEditMode(UAnimInstance* InAnim)
{
    StartEditorPreviewMode();
    FEditorViewportClient* ViewPort = GEngineLoop.GetLevelEditor()->GetViewports()->get();
    DataPreviewController = std::make_shared<FDataPreviewController>(ActiveWorld, ViewPort);
    DataPreviewController->Initialize(InAnim);
}

void UEditorEngine::StartParticleEditMode(UParticleSystemComponent* InParticleComponent)
{
    StartParticlePreviewMode();
    FEditorViewportClient* Viewport = GEngineLoop.GetLevelEditor()->GetViewports()->get();
    SelectedParticleComponent = InParticleComponent;
    ParticlePreviewController = std::make_shared<FParticlePreviewController>(ActiveWorld, Viewport);

    ParticlePreviewController->Initialize(InParticleComponent->Template); // Begin Test

    FParticleEditorPanel* ParticlePanel = dynamic_cast<FParticleEditorPanel*>(GEngineLoop.GetUnrealEditor()->GetEditorPanel("ParticlePanel").get());
    ParticlePanel->SetParticlePreviewController(ParticlePreviewController.get());

    FViewportResource* ViewportResource = ParticlePreviewController->GetViewportClient()->GetViewportResource();
    std::array<float, 4> BlackSceneClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    ViewportResource->SetClearColor(EResourceType::ERT_Scene, BlackSceneClearColor);
    //ViewPort->SetShowFlag()
}

void UEditorEngine::StartParticlePreviewMode()
{ 
    if (ParticlePreviewWorld)
        return;

    FWorldContext& PreviewWorldContext = CreateNewWorldContext(EWorldType::ParticlePreview);

    ParticlePreviewWorld = UWorld::CreateWorld(this, EWorldType::ParticlePreview, TEXT("ParticlePreviewWorld"));

    PreviewWorldContext.SetCurrentWorld(ParticlePreviewWorld);
    ActiveWorld = ParticlePreviewWorld;
}

void UEditorEngine::EndParticlePreviewMode()
{
    if (!ParticlePreviewWorld)
        return;

    if (FWorldContext* Context = GetParticlePreviewWorldContext())
        WorldList.Remove(Context);

    ParticlePreviewWorld->Release();
    GUObjectArray.MarkRemoveObject(ParticlePreviewWorld);
    ParticlePreviewWorld = nullptr;

    ActiveWorld = EditorWorld;

    FEditorViewportClient* Viewport = GEngineLoop.GetLevelEditor()->GetViewports()->get();
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    std::array<float, 4> BaseSceneClearColor = { 0.025f, 0.025f, 0.025f, 1.0f };
    ViewportResource->SetClearColor(EResourceType::ERT_Scene, BaseSceneClearColor);

    ParticlePreviewController = nullptr;
    SelectedParticleComponent->ResetParticleSystem();
    
    SelectedParticleComponent = nullptr;
}

FWorldContext& UEditorEngine::GetEditorWorldContext(/*bool bEnsureIsGWorld*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            return *WorldContext;
        }
    }
    return CreateNewWorldContext(EWorldType::Editor);
}

FWorldContext* UEditorEngine::GetPIEWorldContext(/*int32 WorldPIEInstance*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::PIE)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

FWorldContext* UEditorEngine::GetEditorPreviewWorldContext()
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::EditorPreview)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

FWorldContext* UEditorEngine::GetParticlePreviewWorldContext()
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::ParticlePreview)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

void UEditorEngine::SelectActor(AActor* InActor)
{
    if (InActor && CanSelectActor(InActor))
    {
        PrivateEditorSelection::GActorSelected = InActor;
    }
}

void UEditorEngine::DeselectActor(AActor* InActor)
{
    if (PrivateEditorSelection::GActorSelected == InActor && InActor)
    {
        PrivateEditorSelection::GActorSelected = nullptr;
        ClearComponentSelection();
    }
}

void UEditorEngine::ClearActorSelection()
{
    PrivateEditorSelection::GActorSelected = nullptr;
}

bool UEditorEngine::CanSelectActor(const AActor* InActor) const
{
    return InActor != nullptr && InActor->GetWorld() == ActiveWorld && !InActor->IsActorBeingDestroyed();
}

AActor* UEditorEngine::GetSelectedActor() const
{
    return PrivateEditorSelection::GActorSelected;
}

void UEditorEngine::HoverActor(AActor* InActor)
{
    if (InActor)
    {
        PrivateEditorSelection::GActorHovered = InActor;
    }
}

void UEditorEngine::NewLevel()
{
    ClearActorSelection();
    ClearComponentSelection();

    if (ActiveWorld->GetActiveLevel())
    {
        ActiveWorld->GetActiveLevel()->Release();
    }
}

void UEditorEngine::SelectComponent(USceneComponent* InComponent) const
{
    if (InComponent && CanSelectComponent(InComponent))
    {
        PrivateEditorSelection::GComponentSelected = InComponent;
    }
}

void UEditorEngine::DeselectComponent(USceneComponent* InComponent)
{
    // 전달된 InComponent가 현재 선택된 컴포넌트와 같다면 선택 해제
    if (PrivateEditorSelection::GComponentSelected == InComponent && InComponent != nullptr)
    {
        PrivateEditorSelection::GComponentSelected = nullptr;
    }
}

void UEditorEngine::ClearComponentSelection()
{
    PrivateEditorSelection::GComponentSelected = nullptr;
}

bool UEditorEngine::CanSelectComponent(const USceneComponent* InComponent) const
{
    return InComponent != nullptr && InComponent->GetOwner() && InComponent->GetOwner()->GetWorld() == ActiveWorld && !InComponent->GetOwner()->IsActorBeingDestroyed();
}

USceneComponent* UEditorEngine::GetSelectedComponent() const
{
    return PrivateEditorSelection::GComponentSelected;
}

void UEditorEngine::HoverComponent(USceneComponent* InComponent)
{
    if (InComponent)
    {
        PrivateEditorSelection::GComponentHovered = InComponent;
    }
}

AEditorPlayer* UEditorEngine::GetEditorPlayer() const
{
    return EditorPlayer;
}
