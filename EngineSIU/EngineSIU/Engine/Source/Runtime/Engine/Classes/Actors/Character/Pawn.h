#pragma once
#include "GameFramework/Actor.h"

class USkeletalMeshComponent;

enum EMovementMode
{
    EIdle,
    EWalking,
    EFlying,
    EDancing,
    EDie,
};

class APawn : public AActor
{
    DECLARE_CLASS(APawn, AActor)
public:
    APawn();
    virtual ~APawn() = default;

    virtual void PostSpawnInitialize() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    EMovementMode CurrentMovementMode = EIdle;

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    float GetExciting() const { return Exciting;}
    void SetExciting(float InExciting) { Exciting = InExciting; }
    
    float Exciting = 0.f;
    
protected:
    UPROPERTY
    (USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr)
};

