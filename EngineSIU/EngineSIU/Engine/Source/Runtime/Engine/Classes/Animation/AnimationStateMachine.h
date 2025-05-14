#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "sol/sol.hpp"

struct FBonePose;
class APawn;
class UAnimInstance;

enum EAnimState
{
    AS_Idle,
    AS_Work,
    AS_Run,
    AS_Fly,
    AS_Dance,
    AS_Die,
};

class UAnimationStateMachine : public UObject
{
    DECLARE_CLASS(UAnimationStateMachine, UObject)

public:
    UAnimationStateMachine() = default;
    virtual ~UAnimationStateMachine();
    // LuaScriptName은 기본 경로 "Scripts/Animation/"에 추가.
    // 예: "MyScript.lua" -> "Scripts/Animation/MyScript.lua"
    // 인자는 하위 폴더 경로 + 파일명.lua 만 넣어줄 것.
    void Initialize(APawn* InOwner, const FString& LuaScriptName, UAnimInstance* InAnimInstance);

    void ProcessState();

    APawn* Owner;

public:
    void InitLuaStateMachine();

    void SetScriptFilePath(const FString& InScriptFilePath) { ScriptFilePath = InScriptFilePath; }

    FString GetScriptFilePath() const { return ScriptFilePath; }

private:
    FString ScriptFilePath;
    sol::table LuaTable;

    UAnimInstance* OwnedAnimInstance;

    FString LastStateName;
    
};
