#pragma once
#include "Actors/Player.h"

class UFishBodyComponent;
class USphereComponent;
class UStaticMeshComponent;
class UFishTailComponent;

DECLARE_DELEGATE_TwoParams(FOnFishHealthChanged, int32 /* CurrentHealth */, int32 /* MaxHealth */);

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
    void SetHealth(int32 InHealth);

    int32 GetMaxHealth() const { return MaxHealth; }
    void SetMaxHealth(int32 InMaxHealth) { MaxHealth = InMaxHealth; }

    float GetHealthPercent() const { return static_cast<float>(Health) / static_cast<float>(MaxHealth); }

    FOnFishHealthChanged OnHealthChanged;
    
protected:
    UPROPERTY
    (USphereComponent*, SphereComponent, = nullptr)

    UPROPERTY
    (UFishBodyComponent*, FishBody, = nullptr)

    UPROPERTY
    (UFishTailComponent*, FishTail, = nullptr)

    FVector Velocity = FVector::ZeroVector;

    float JumpZVelocity;

    float Gravity;

    void Move(float DeltaTime);

    void RotateMesh();

    float MeshPitchMax;

    float MeshPitch;
    
    float MeshRotSpeed = 10.f;

    int32 MaxHealth;

    int32 Health;

    void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
