#include "PlayerController.h"

#include "Camera/PlayerCameraManager.h"
#include "UObject/UObjectIterator.h"



APlayerController::APlayerController()
{
    SetupInputComponent();
}


APlayerController::~APlayerController()
{
}

void APlayerController::BeginPlay()
{
}

void APlayerController::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
    
    if (bHasPossessed)
    {
        ProcessInput(DeltaTime);
    }

    if (PlayerCameraManager)
    {
        //PlayerCameraManager->UpdateCamera(DeltaTime);
    }

}

void APlayerController::ProcessInput(float DeltaTime) const
{
    if (InputComponent)
    {
        InputComponent->ProcessInput(DeltaTime);
    }
}

void APlayerController::Destroyed()
{
}

void APlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void APlayerController::Possess(AActor* InActor)
{
    CurrentPossess = InActor;
    bHasPossessed = true;

    if (InputComponent)
    {
        InputComponent->SetPossess();
    }
}

void APlayerController::UnPossess()
{
    CurrentPossess = nullptr;
    bHasPossessed = false;

    if (InputComponent)
    {
        InputComponent->UnPossess();
    }
}

void APlayerController::SetupInputComponent()
{
    // What is the correct parameter of ConstructObject?
    if (InputComponent == nullptr) {
        InputComponent = FObjectFactory::ConstructObject<UInputComponent>(this);
    }
}

void APlayerController::BindAction(const FString& Key, const std::function<void(float)>& Callback)
{
    if (InputComponent)
    {
        InputComponent->BindAction(Key, Callback);
    }
}

AActor* APlayerController::GetViewTarget() const
{
    AActor* CameraManagerViewTarget = PlayerCameraManager ? PlayerCameraManager->GetViewTarget() : nullptr;

    return CameraManagerViewTarget ? CameraManagerViewTarget : const_cast<APlayerController*>(this);
}
