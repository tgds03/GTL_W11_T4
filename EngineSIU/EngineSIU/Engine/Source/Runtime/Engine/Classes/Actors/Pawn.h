#pragma once
#include "GameFramework/Actor.h"

class USkeletalMeshComponent;

enum EMovementMode
{
    EIdle,
    EWalking,
    EFlying,
    EDancing,
};

class APawn : public AActor
{
    DECLARE_CLASS(APawn, AActor)
public:
    APawn();

    EMovementMode CurrentMovementMode = EIdle;
protected:
    UPROPERTY
    (USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr)
};

