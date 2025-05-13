#include "Pawn.h"

#include "Components/SkeletalMesh/SkeletalMeshComponent.h"

APawn::APawn()
{
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>();
    SetRootComponent(SkeletalMeshComponent);
    SkeletalMeshComponent->InitializeAnimInstance(this);
    SetActorTickInEditor(true);
}

