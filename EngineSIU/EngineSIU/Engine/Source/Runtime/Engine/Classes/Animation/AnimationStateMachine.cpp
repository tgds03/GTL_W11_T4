#include "AnimationStateMachine.h"

#include "Actors/Character/Pawn.h"

void UAnimationStateMachine::Initialize(APawn* InOwner)
{
    Owner = InOwner;
}

void UAnimationStateMachine::ProcessState()
{
    if (Owner->CurrentMovementMode == EIdle)
    {
        CurrentState = AS_Idle;
    }
    else if (Owner->CurrentMovementMode == EDancing)
    {
        CurrentState = AS_Dance;
    }
    else if (Owner->CurrentMovementMode == EDie)
    {
        CurrentState = AS_Die;
    }
}
