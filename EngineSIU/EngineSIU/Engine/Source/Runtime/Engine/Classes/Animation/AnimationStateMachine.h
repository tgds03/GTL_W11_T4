#pragma once
#include "UObject/Object.h"

class APawn;

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
public:

    virtual void Initialize(APawn* InOwner);
    
    void ProcessState();

    EAnimState CurrentState;

    APawn* Owner;
};
