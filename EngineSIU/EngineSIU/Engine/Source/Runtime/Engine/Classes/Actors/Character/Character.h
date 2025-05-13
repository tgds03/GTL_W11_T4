#pragma once
#include "Pawn.h"

class FAnimNotifyEvent;

class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)
public:
    ACharacter();
    virtual ~ACharacter() = default;
    virtual void PostSpawnInitialize() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    void HandleAnimNotify(const FAnimNotifyEvent* NotifyEvent);
};

