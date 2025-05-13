#include "Animation/AnimTypes.h"
#include "Character.h"

ACharacter::ACharacter()
{
}

void ACharacter::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
}

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewCharacter = Cast<ThisClass>(Super::Duplicate(InOuter));
    return NewCharacter;
}

void ACharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ACharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACharacter::Destroyed()
{
    Super::Destroyed();
}

void ACharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay    (EndPlayReason);
}

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent* Notify)
{
    if (Notify->NotifyName == TEXT("Twerk"))
    {
        // Handle the Twerk animation notify
    }
    else
    {
        // Default or unknown notify handling
    }
}
