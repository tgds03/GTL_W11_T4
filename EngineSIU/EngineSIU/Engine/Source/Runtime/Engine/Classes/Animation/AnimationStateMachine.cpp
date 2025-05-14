#include "AnimationStateMachine.h"

#include "AnimSequence.h"
#include "Actors/Character/Pawn.h"
#include "Engine/Lua/LuaScriptManager.h"
#include "Animation/UAnimInstance.h"

UAnimationStateMachine::~UAnimationStateMachine()
{
    FLuaScriptManager::Get().UnRegisterActiveAnimLua(this);
    if (LuaTable.valid())
    {
        LuaTable.clear();
        LuaTable = sol::table();
    }
}

void UAnimationStateMachine::Initialize(APawn* InOwner, const FString& LuaScriptName, UAnimInstance* InAnimInstance)
{
    Owner = InOwner;
    ScriptFilePath = LuaScriptName;
    OwnedAnimInstance = InAnimInstance;
    LastStateName = TEXT("");

    InitLuaStateMachine();
}

void UAnimationStateMachine::ProcessState()
{
    if (!LuaTable.valid())
        return;

    sol::function UpdateFunc = LuaTable["Update"];
    if (!UpdateFunc.valid())
    {
        UE_LOG(LogLevel::Warning, TEXT("Lua Update function not valid!"));
        return;
    }
    
    sol::object result = UpdateFunc(LuaTable, 0.0f);

    sol::table StateInfo = result.as<sol::table>();
    FString StateName = StateInfo["anim"].get_or(std::string("")).c_str();
    float Blend = StateInfo["blend"].get_or(0.f);

    if (!StateName.IsEmpty() && StateName != LastStateName)
    {
        LastStateName = StateName;

        // 애니메이션 이름을 AnimInstance에 전달
        if (OwnedAnimInstance)
        {
            UAnimSequence* Sequence = FResourceManager::LoadAnimationSequence(StateName);
            if (Sequence)
            {   
                OwnedAnimInstance->SetTargetSequence(Sequence, Blend);
            }
            else
            {
                UE_LOG(LogLevel::Display, TEXT("AnimSequence not found for state: %s"), *StateName);
            }
        }
    }
}

void UAnimationStateMachine::InitLuaStateMachine()
{
    LuaTable = FLuaScriptManager::Get().CreateLuaTable(ScriptFilePath);

    FLuaScriptManager::Get().RegisterActiveAnimLua(this);
    if (!LuaTable.valid())
        return;
    LuaTable["Owner"] = Owner;
}


