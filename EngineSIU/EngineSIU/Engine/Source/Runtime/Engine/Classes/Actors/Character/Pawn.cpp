#include "Pawn.h"

#include "Components/SkeletalMesh/SkeletalMeshComponent.h"

APawn::APawn()
{
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>();
    SetRootComponent(SkeletalMeshComponent);
    SkeletalMeshComponent->InitializeAnimInstance(this);
    SetActorTickInEditor(true);
}

void APawn::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
}

UObject* APawn::Duplicate(UObject* InOuter)
{
    return nullptr;
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

