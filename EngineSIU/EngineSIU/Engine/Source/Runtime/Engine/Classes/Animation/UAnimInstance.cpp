#include "UAnimInstance.h"

void UAnimInstance::Initialize(USkeletalMeshComponent* InComponent)
{
    OwningComponent = InComponent;
    CurrentTime = 0.0f;
    PlayRate = 1.0f;
}

void UAnimInstance::SetAnimSequence(UAnimSequence* InSequence)
{
    CurrentSequence = InSequence;
    CurrentTime = 0.0f; // 애니메이션 시간 초기화
}
