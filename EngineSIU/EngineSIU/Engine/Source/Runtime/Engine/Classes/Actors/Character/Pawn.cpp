#include "Pawn.h"

#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "Components/LuaScriptComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"

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

    if (LuaScriptComponent)
    {
        LuaScriptComponent->SetScriptName(TEXT("Scripts/DefaultPawn.lua"));
    }
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

void APawn::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(APawn, sol::bases<AActor>())
}

bool APawn::BindSelfLuaProperties()
{
    Super::BindSelfLuaProperties();

    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }
    LuaTable["this"] = this;

    return true;
}

