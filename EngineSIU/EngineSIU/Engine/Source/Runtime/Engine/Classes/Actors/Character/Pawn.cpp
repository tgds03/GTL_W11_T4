#include "Pawn.h"

#include "Components/SkeletalMesh/SkeletalMeshComponent.h"

APawn::APawn()
{
}

void APawn::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>();
    SetRootComponent(SkeletalMeshComponent);
    SkeletalMeshComponent->InitializeAnimInstance(this);
    SetActorTickInEditor(true);
}

UObject* APawn::Duplicate(UObject* InOuter)
{
    ThisClass* NewPawn = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewPawn->CurrentMovementMode = CurrentMovementMode;
    NewPawn->SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>();

    return NewPawn;
}

void APawn::BeginPlay()
{
    Super::BeginPlay();
}

void APawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APawn::Destroyed()
{
    Super::Destroyed();
}

void APawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

