#include "SpringArmComponent.h"

#include "Actor.h"
#include "HAL/PlatformType.h"

USpringArmComponent::USpringArmComponent()
{
    TargetArmLength = 5.f; 

    bUsePawnControlRotation = false;
    bDoCollisionTest = false;

    bInheritPitch = bInheritYaw = bInheritRoll = true;
    bEnableCameraLag = true;
    bEnableCameraRotationLag = true;
    bUseCameraLagSubstepping = true;

    ProbeSize = 12.0f;
    CameraLagSpeed = 10.f;
    CameraRotationLagSpeed = 10.f;
    CameraLagMaxTimeStep = 1.f / 60.f;
    CameraLagMaxDistance = 0.f;
}

void USpringArmComponent::TickComponent(float DeltaTime)
{

    //Super::TickComponent(DeltaTime);
    UpdateDesiredArmLocation(bDoCollisionTest, bEnableCameraLag, bEnableCameraRotationLag, DeltaTime);
}

void USpringArmComponent::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
    FRotator DesiredRot = GetTargetRotation();
    //UE_LOG(LogLevel::Display, TEXT("Target Rotation : %.2f %.2f %.2f"), DesiredRot.Yaw, DesiredRot.Pitch, DesiredRot.Roll);

    /* 회전 지연 여부 검사 및 반영 : bDoRotationLag */
    /* 서브스텝 분기 및 단일 스텝 분기 (DeltaTime 전체를 여러 LagMaxTimeStep이하 조각으로 나누고, 각 조각마다 RInterTo를 호출 */
    if (bDoRotationLag)
    {
        if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
        {
            const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
            FRotator LerpTarget = PreviousDesiredRot;
            float RemainingTime = DeltaTime;
            while (RemainingTime > KINDA_SMALL_NUMBER)
            {
                const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
                LerpTarget += ArmRotStep * LerpAmount;
                RemainingTime -= LerpAmount;

                DesiredRot = FMath::RInterpTo(PreviousDesiredRot, LerpTarget, LerpAmount, CameraRotationLagSpeed);
                PreviousDesiredRot = DesiredRot;
            }
        }
        else
        {
            DesiredRot = FMath::RInterpTo(PreviousDesiredRot, DesiredRot, DeltaTime, CameraRotationLagSpeed);
        }
    }

    /* 위치 지연 여부 검사 및 반영 : bDoLocationLag */
    FVector ArmOrigin = GetWorldLocation() + TargetOffset;
    FVector DesiredLoc = ArmOrigin;


    if (bDoLocationLag)
    {
        if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
        {
            const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
            FVector LerpTarget = PreviousDesiredLoc;

            float RemainingTime = DeltaTime;
            while (RemainingTime > KINDA_SMALL_NUMBER)
            {
                const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
                LerpTarget += ArmMovementStep * LerpAmount;
                RemainingTime -= LerpAmount;

                DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
                PreviousDesiredLoc = DesiredLoc;
            }
        }
        else
        {
            DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
        }
    }
    //PreviousArmOrigin = ArmOrigin;
    //PreviousDesiredLoc = DesiredLoc; // 예외 처리일라나?

    DesiredLoc -= DesiredRot.ToVector() * TargetArmLength;
    //DesiredLoc += FMatrix::GetRotationMatrix(DesiredRot).TransformVector(SocketOffset); // [unused] 로컬 공간 오프셋 추가

    //FTransform WorldCamTM;
    // 반영 및 child transform 반영


    FVector ResultLoc;
    /* 충돌 검사 및 암 길이 줄일지 여부 결정 : bDoTrace */
    if (bDoTrace)
    {
        //ResultLoc = BlendLocations(DesiredLoc, ArmOrigin, false, DeltaTime);
    }
    else
    {
        ResultLoc = DesiredLoc;
    }

    SetWorldLocation(ResultLoc);
    SetWorldRotation(DesiredRot);
}

FVector USpringArmComponent::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime)
{
    return bHitSomething ? TraceHitLocation : DesiredArmLocation;
}

FRotator USpringArmComponent::GetDesiredRotation() const
{
    return GetWorldRotation();
}

FRotator USpringArmComponent::GetTargetRotation() const
{
    FRotator DesiredRot = GetDesiredRotation();

    if (AActor* OwningActor = Cast<AActor>(GetOwner()))
    {
        if (bUsePawnControlRotation)
        {
            const FRotator ActorViewRoation = OwningActor->GetActorRotation();
            DesiredRot = ActorViewRoation;
        }
    }

    /* 만일 월드 기준 절대 회전 값이 아닌 상대 회전 값을 쓴다면
     * binheritPitch, bInheritYaw, bInheritRoll를 검사하여
     * DesiredRot = LocalRelativeRotation으로 변경
     */

    return DesiredRot;
}
