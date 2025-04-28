#pragma once
#include "Actors/Player.h"

class UFishBodyComponent;
class USphereComponent;
class UStaticMeshComponent;
class UFishTailComponent;

class AFish : public APlayer
{
    DECLARE_CLASS(AFish, APlayer)

public:
    AFish();
    virtual ~AFish() override = default;

    virtual void PostSpawnInitialize() override;

    UObject* Duplicate(UObject* InOuter) override;

    void BeginPlay() override;

    void Tick(float DeltaTime) override;

    int32 GetHealth() const { return Health; }
    void SetHealth(int32 InHealth) { Health = FMath::Max(0, FMath::Min(InHealth, MaxHealth)); }

    int32 GetMaxHealth() const { return MaxHealth; }
    void SetMaxHealth(int32 InMaxHealth) { MaxHealth = InMaxHealth; }

    float GetHealthPercent() const { return static_cast<float>(Health) / static_cast<float>(MaxHealth); }

protected:
    UPROPERTY
    (USphereComponent*, SphereComponent, = nullptr)

    UPROPERTY
    (UFishBodyComponent*, FishBody, = nullptr)

    UPROPERTY
    (UFishTailComponent*, FishTail, = nullptr)

    FVector Velocity = FVector::ZeroVector;

    float JumpZVelocity = 50.f;

    float Gravity = -9.8f * 10.f;

    void Move(float DeltaTime);

    void RotateMesh();

    float MeshPitchMax = 15.f;

    float MeshRotSpeed = 10.f;

    int32 Health = 10;

    int32 MaxHealth = 10;

    void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
