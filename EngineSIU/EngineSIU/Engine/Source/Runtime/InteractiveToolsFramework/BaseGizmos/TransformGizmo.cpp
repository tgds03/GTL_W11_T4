#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "Define.h"
#include "GizmoCircleComponent.h"
#include "Actors/Player.h"
#include "GizmoRectangleComponent.h"
#include "Engine/EditorEngine.h"
#include "World/World.h"
#include "Engine/FObjLoader.h"

ATransformGizmo::ATransformGizmo()
{
    static int a = 0;
    UE_LOG(LogLevel::Error, "Gizmo Created %d", a++);
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoTranslationX.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoTranslationY.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoTranslationZ.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoRotationX.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoRotationY.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoRotationZ.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoScaleX.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoScaleY.obj");
    FResourceManager::CreateStaticMesh("Assets/Gizmo/GizmoScaleZ.obj");

    SetRootComponent(
        AddComponent<USceneComponent>()
    );

    UGizmoArrowComponent* locationX = AddComponent<UGizmoArrowComponent>();
    locationX->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoTranslationX.obj"));
    locationX->SetupAttachment(RootComponent);
    locationX->SetGizmoType(UGizmoBaseComponent::ArrowX);
    ArrowArr.Add(locationX);

    UGizmoArrowComponent* locationY = AddComponent<UGizmoArrowComponent>();
    locationY->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoTranslationY.obj"));
    locationY->SetupAttachment(RootComponent);
    locationY->SetGizmoType(UGizmoBaseComponent::ArrowY);
    ArrowArr.Add(locationY);

    UGizmoArrowComponent* locationZ = AddComponent<UGizmoArrowComponent>();
    locationZ->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoTranslationZ.obj"));
    locationZ->SetupAttachment(RootComponent);
    locationZ->SetGizmoType(UGizmoBaseComponent::ArrowZ);
    ArrowArr.Add(locationZ);

    UGizmoRectangleComponent* ScaleX = AddComponent<UGizmoRectangleComponent>();
    ScaleX->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoScaleX.obj"));
    ScaleX->SetupAttachment(RootComponent);
    ScaleX->SetGizmoType(UGizmoBaseComponent::ScaleX);
    RectangleArr.Add(ScaleX);

    UGizmoRectangleComponent* ScaleY = AddComponent<UGizmoRectangleComponent>();
    ScaleY->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoScaleY.obj"));
    ScaleY->SetupAttachment(RootComponent);
    ScaleY->SetGizmoType(UGizmoBaseComponent::ScaleY);
    RectangleArr.Add(ScaleY);

    UGizmoRectangleComponent* ScaleZ = AddComponent<UGizmoRectangleComponent>();
    ScaleZ->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoScaleZ.obj"));
    ScaleZ->SetupAttachment(RootComponent);
    ScaleZ->SetGizmoType(UGizmoBaseComponent::ScaleZ);
    RectangleArr.Add(ScaleZ);

    UGizmoCircleComponent* CircleX = AddComponent<UGizmoCircleComponent>();
    CircleX->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoRotationX.obj"));
    CircleX->SetupAttachment(RootComponent);
    CircleX->SetGizmoType(UGizmoBaseComponent::CircleX);
    CircleArr.Add(CircleX);

    UGizmoCircleComponent* CircleY = AddComponent<UGizmoCircleComponent>();
    CircleY->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoRotationY.obj"));
    CircleY->SetupAttachment(RootComponent);
    CircleY->SetGizmoType(UGizmoBaseComponent::CircleY);
    CircleArr.Add(CircleY);

    UGizmoCircleComponent* CircleZ = AddComponent<UGizmoCircleComponent>();
    CircleZ->SetStaticMesh(FResourceManager::GetStaticMesh(L"Assets/Gizmo/GizmoRotationZ.obj"));
    CircleZ->SetupAttachment(RootComponent);
    CircleZ->SetGizmoType(UGizmoBaseComponent::CircleZ);
    CircleArr.Add(CircleZ);
}

void ATransformGizmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Editor 모드에서만 Tick.
    if (GEngine->ActiveWorld->WorldType != EWorldType::Editor)
    {
        return;
    }

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        return;
    }
    AEditorPlayer* EditorPlayer = Engine->GetEditorPlayer();
    if (!EditorPlayer)
    {
        return;
    }
    
    USceneComponent* SelectedComponent = Engine->GetSelectedComponent();
    AActor* SelectedActor = Engine->GetSelectedActor();

    USceneComponent* TargetComponent = nullptr;

    if (SelectedComponent != nullptr)
    {
        TargetComponent = SelectedComponent;
    }
    else if (SelectedActor != nullptr)
    {
        TargetComponent = SelectedActor->GetRootComponent();
    }

    if (TargetComponent)
    {
        SetActorLocation(TargetComponent->GetWorldLocation());
        if (EditorPlayer->GetCoordMode() == ECoordMode::CDM_LOCAL || EditorPlayer->GetControlMode() == EControlMode::CM_SCALE)
        {
            SetActorRotation(TargetComponent->GetWorldRotation());
        }
        else
        {
            SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
        }
    }
}

void ATransformGizmo::Initialize(FEditorViewportClient* InViewport)
{
    AttachedViewport = InViewport;
}
