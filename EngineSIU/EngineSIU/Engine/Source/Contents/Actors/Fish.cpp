
#include "Fish.h"

#include "ItemActor.h"
#include "PlatformActor.h"
#include "Components/SphereComponent.h"
#include "Contents/Components/FishTailComponent.h"
#include "Contents/Components/FishBodyComponent.h"
#include "Engine/FObjLoader.h"

AFish::AFish()
    : JumpZVelocity(50.f)
    , Gravity(-9.8f * 10.f)
    , bShouldApplyGravity(true)
    , MeshPitchMax(5.f)
    , MeshPitch(MeshPitchMax)
    , MaxHealth(10)
    , Health(MaxHealth)
{
    
}

void AFish::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    
    SphereComponent = AddComponent<USphereComponent>(FName("SphereComponent_0"));
    SetRootComponent(SphereComponent);

    FishBody = AddComponent<UFishBodyComponent>(FName("FishBodyComponent_0"));
    FishBody->SetupAttachment(SphereComponent);
    
    FishTail = AddComponent<UFishTailComponent>(FName("FishTailComponent_0"));
    FishTail->SetupAttachment(FishBody);
}

UObject* AFish::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}

void AFish::BeginPlay()
{
    APlayer::BeginPlay();

    Velocity = FVector(0.f, 0.f, JumpZVelocity);

    OnActorBeginOverlap.AddLambda(
        [this](AActor* OverlappedActor, AActor* OtherActor)
        {
            ActorBeginOverlap(OverlappedActor, OtherActor);
        }
    );

    OnHealthChanged.AddLambda(
        [this](int32 InCurrentHealth, int32 InMaxHealth)
        {
            const float HealthPercent = static_cast<float>(InCurrentHealth) / static_cast<float>(InMaxHealth);
            if (UFishTailComponent* TailComp = GetComponentByClass<UFishTailComponent>())
            {
                TailComp->SetCurrentYaw(TailComp->GetMaxYaw() * HealthPercent);
                TailComp->SetCurrentFrequency(TailComp->GetMaxFrequency() * HealthPercent);
            }

            MeshPitch = MeshPitchMax * HealthPercent;
        }
    );

    OnDied.AddLambda(
        [this]()
        {
            if (UFishBodyComponent* MeshComp = GetComponentByClass<UFishBodyComponent>())
            {
                MeshComp->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/FishDish/FishDish.obj"));
            }
        }
    );
}

void AFish::Tick(float DeltaTime)
{
    APlayer::Tick(DeltaTime);

    if (bShouldApplyGravity)
    {
        Move(DeltaTime);
    }

    RotateMesh();
}

void AFish::SetHealth(int32 InHealth)
{
    Health = FMath::Max(0, FMath::Min(InHealth, MaxHealth));

    OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth());

    if (IsDead())
    {
        OnDied.Broadcast();
    }
}

void AFish::Move(float DeltaTime)
{
    FVector NextVelocity = Velocity;
    NextVelocity.Z += Gravity * DeltaTime;

    Velocity = NextVelocity;
    
    FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;
    SetActorLocation(NewLocation);
}

void AFish::RotateMesh()
{
    const float VelocityZ = Velocity.Z;

    float RotFactor = FMath::Clamp(VelocityZ, -MeshRotSpeed, MeshRotSpeed);
    
    if (UFishBodyComponent* MeshComp = GetComponentByClass<UFishBodyComponent>())
    {
        // 현재 PIE 모드에서 맴버 변수를 접근할 수 없기 때문에 이렇게 접근 함.
        FRotator CompRotation = MeshComp->GetRelativeRotation();
        CompRotation.Roll = RotFactor * MeshPitch * -1.f;
        
        MeshComp->SetRelativeRotation(CompRotation);
    }
}

void AFish::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (OtherActor->IsA<APlatformActor>())
    {
        if (IsDead())
        {
            Velocity.Z = 0.f;
            bShouldApplyGravity = false;
        }
        else
        {
            Velocity.Z = JumpZVelocity;
        }

        SetHealth(GetHealth() - 1);
    }
    else if (OtherActor->IsA<AItemActor>())
    {
        
    }
}
