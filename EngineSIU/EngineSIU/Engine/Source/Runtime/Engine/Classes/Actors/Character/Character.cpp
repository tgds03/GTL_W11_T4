#include "Character.h"
#include "Animation/AnimTypes.h"

#include "Components/LuaScriptComponent.h"
#include "Engine/Lua/LuaUtils/LuaTypeMacros.h"

ACharacter::ACharacter()
{
}

void ACharacter::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
}

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewCharacter = Cast<ThisClass>(Super::Duplicate(InOuter));
    return NewCharacter;
}

void ACharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ACharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACharacter::Destroyed()
{
    Super::Destroyed();
}

void ACharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay    (EndPlayReason);
}

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent* Notify)
{
    if (Notify->NotifyName == TEXT("Attack"))
    {
        std::cout << "Attack Yap!" << std::endl;
    }
    else
    {
        // Default or unknown notify handling
    }
}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor, APawn>())
}

bool ACharacter::BindSelfLuaProperties()
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
