#include "PlayerCameraManager.h"

#include "CameraModifier.h"
#include "GameFramework/PlayerController.h"
#include "World/World.h"

bool FTViewTarget::Equal(const FTViewTarget& OtherTarget) const
{
    //@TODO: Should I compare Controller too?
    return (Target == OtherTarget.Target) && POV.Equals(OtherTarget.POV);
}

void FTViewTarget::CheckViewTarget(APlayerController* OwningController)
{
    if (Target == nullptr)
    {
        Target = OwningController;
    }

    if (Target == OwningController->GetPossessedActor())
    {
        return;
    }
    
    if (Target != nullptr)
    {
        // PossessActor가 있을때
        if (OwningController->GetPossessedActor() && !OwningController->GetPossessedActor()->IsActorBeingDestroyed() )
        {
            OwningController->PlayerCameraManager->AssignViewTarget(OwningController->GetPossessedActor(), *this);
        }
        else
        {
            OwningController->PlayerCameraManager->AssignViewTarget(OwningController, *this);
        }
    }
}

void FTViewTarget::SetNewTarget(AActor* NewTarget)
{
    Target = NewTarget;
}

AActor* FTViewTarget::GetTargetActor() const
{
    if (Target)
    {
        return Target;
    }

    if (APlayerController* Controller = Cast<APlayerController>(Target))
    {
        return Controller->GetPossessedActor();
    }

    return nullptr;
}

APlayerCameraManager::APlayerCameraManager()
{
    DefaultFOV = 90.0f;
    DefaultAspectRatio = 1.33333f;
    bDefaultConstrainAspectRatio = false;

    ViewPitchMin = -89.9f;
    ViewPitchMax = 89.9f;
    ViewYawMin = 0.f;
    ViewYawMax = 359.999f;
    ViewRollMin = -89.9f;
    ViewRollMax = 89.9f;
}

void APlayerCameraManager::InitializeFor(APlayerController* PC)
{
    PCOwner = PC;
}

AActor* APlayerCameraManager::GetViewTarget() const
{
    return ViewTarget.Target;
}

void APlayerCameraManager::UpdateCamera(float DeltaTime)
{
    if (PCOwner)
    {
        DoUpdateCamera(DeltaTime);
    }
}

/**
 * @param FromAlpha          시작 알파값 (0: 완전 투명, 1: 완전 불투명)
 * @param ToAlpha            목표 알파값
 * @param InFadeTime         페이드 지속 시간 (초 단위)
 * @param InFadeColor        페이드에 사용될 색상
 * @param bInHoldWhenFinished 페이드 종료 후의 화면 유지 여부
 */
void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float InFadeTime, FLinearColor InFadeColor, bool bInHoldWhenFinished)
{
    bEnableFading = true;

    FadeColor = InFadeColor;
    FadeAlpha = FVector2D(FromAlpha, ToAlpha);
    FadeTime = InFadeTime;
    FadeTimeRemaining = InFadeTime;

    /* 오디오 효과 중략 */

    bHoldFadeWhenFinished = bInHoldWhenFinished;
}

void APlayerCameraManager::StopCameraFade()
{    
    if (bEnableFading == true)
    {
        // Make sure FadeAmount finishes at the desired value
        FadeAmount = FadeAlpha.Y;
        bEnableFading = false;
        //StopAudioFade();
    }
}

void APlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
    TArray<UCameraModifier*> LocalModifierList = ModifierList;

    for (int32 ModifierIdx = 0; ModifierIdx < LocalModifierList.Num(); ++ModifierIdx)
    {
        bool bContinue = true;

        UCameraModifier* CameraModifier = LocalModifierList[ModifierIdx];
        if (CameraModifier)
        {
            bContinue = !CameraModifier->ModifyCamera(DeltaTime, InOutPOV);
        }

        if (!bContinue)
        {
            return;
        }
    }
}

void APlayerCameraManager::DoUpdateCamera(float DeltaTime)
{
    FMinimalViewInfo NewPOV = ViewTarget.POV;

    //Progress가 진행중이 아니면
    if (PendingViewTarget.Target == nullptr)
    {
        ViewTarget.CheckViewTarget(PCOwner);
        UpdateViewTarget(ViewTarget, DeltaTime);
    }

    NewPOV = ViewTarget.POV;
    
    if (PendingViewTarget.Target != nullptr)
    {
        BlendTimeToGo -= DeltaTime;
        //PendingViewTarget은 움직여야할 목표가 아니라 얘를 움직여야함.
        //그래서 얘를 ViewTarget으로 설정해줌
        PendingViewTarget.CheckViewTarget(PCOwner);
        UpdateViewTarget(PendingViewTarget, DeltaTime);

        if (BlendTimeToGo > 0)
        {
            float DurationPct = (BlendParams.BlendTime - BlendTimeToGo) / BlendParams.BlendTime;

            float BlendPct = 0.f;
            switch (BlendParams.BlendFunction)
            {
            case VTBlend_Linear:
                BlendPct = FMath::Lerp(0.f, 1.f, DurationPct);
                break;
            case VTBlend_Cubic:
                BlendPct = FMath::CubicInterp(0.f, 0.f, 1.f, 0.f, DurationPct);
                break;
            case VTBlend_EaseIn:
                BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, BlendParams.BlendExp));
                break;
            case VTBlend_EaseOut:
                BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, 1.f / BlendParams.BlendExp));
                break;
            case VTBlend_EaseInOut:
                BlendPct = FMath::InterpEaseInOut(0.f, 1.f, DurationPct, BlendParams.BlendExp);
                break;
            case VTBlend_PreBlended:
                BlendPct = 1.0f;
                break;
            default:
                break;
            }

            // Update pending view target blend
            NewPOV = ViewTarget.POV;
            //@TODO: CAMERA: Make sure the sense is correct!  BlendViewTargets(ViewTarget, PendingViewTarget, BlendPct);
            NewPOV.BlendViewInfo(PendingViewTarget.POV, BlendPct);
        }
        else
        {
            // we're done blending, set new view target
            ViewTarget = PendingViewTarget;

            // clear pending view target
            PendingViewTarget.Target = nullptr;

            BlendTimeToGo = 0;

            // our camera is now viewing there
            NewPOV = PendingViewTarget.POV;

            OnBlendCompleteEvent.Broadcast();
        }
    }
    
    // Fade Enabled 되었다면 Fade 처리 수행
    if (bEnableFading)
    {
        FadeTimeRemaining = FMath::Max(FadeTimeRemaining - DeltaTime, 0.0f);
        if (FadeTime > 0.0f)
        {
            FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining / FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
        }

        if (/*(bHoldFadeWhenFinished == false) && */(FadeTimeRemaining <= 0.f))
        {
            // done
            StopCameraFade();
        }
    }

    FillCameraCache(NewPOV);
}

void APlayerCameraManager::SetViewTarget(class AActor* NewTarget, struct FViewTargetTransitionParams TransitionParams)
{
	// Make sure view target is valid
	if (NewTarget == nullptr)
	{
		NewTarget = PCOwner;
	}

	// Update current ViewTargets
	ViewTarget.CheckViewTarget(PCOwner);
	if (PendingViewTarget.Target)
	{
		PendingViewTarget.CheckViewTarget(PCOwner);
	}

	// If we're already transitioning to this new target, don't interrupt.
	if (PendingViewTarget.Target != nullptr && NewTarget == PendingViewTarget.Target)
	{
		return;
	}

	// if viewtarget different then new one or we're transitioning from the same target with locked outgoing, then assign it
	if ((NewTarget != ViewTarget.Target) || (PendingViewTarget.Target))
	{
		// if a transition time is specified, then set pending view target accordingly
		if (TransitionParams.BlendTime > 0)
		{
			// band-aid fix so that EndViewTarget() gets called properly in this case
			if (PendingViewTarget.Target == nullptr)
			{
				PendingViewTarget.Target = ViewTarget.Target;
			}

			// use last frame's POV
			// ViewTarget.POV = GetLastFrameCameraCacheView();
			BlendTimeToGo = TransitionParams.BlendTime;

			AssignViewTarget(NewTarget, PendingViewTarget, TransitionParams);
			PendingViewTarget.CheckViewTarget(PCOwner);

		}
		else
		{
			// otherwise, assign new viewtarget instantly
			AssignViewTarget(NewTarget, ViewTarget);
			ViewTarget.CheckViewTarget(PCOwner);
			// remove old pending ViewTarget so we don't still try to switch to it
			PendingViewTarget.Target = nullptr;
		}
	}
	else
	{
		// we're setting the viewtarget to the viewtarget we were transitioning away from,
		// just abort the transition.
		// @fixme, investigate if we want this case to go through the above code, so AssignViewTarget et al
		// get called
		if (PendingViewTarget.Target != nullptr)
		{
			if (!PCOwner->IsActorBeingDestroyed())
			{
				PCOwner->ClientSetViewTarget(NewTarget, TransitionParams);
			}
		}
		PendingViewTarget.Target = nullptr;
	}

	// update the blend params after all the assignment logic so that sub-classes can compare
	// the old vs new parameters if needed.
	BlendParams = TransitionParams;
}

void APlayerCameraManager::SetCameraCachePOV(const FMinimalViewInfo& InPOV)
{
    CameraCachePrivate.POV = InPOV;
}

void APlayerCameraManager::SetLastFrameCameraCachePOV(const FMinimalViewInfo& InPOV)
{
    LastFrameCameraCachePrivate.POV = InPOV;
}

const FMinimalViewInfo& APlayerCameraManager::GetCameraCacheView() const
{
    return CameraCachePrivate.POV;
}

const FMinimalViewInfo& APlayerCameraManager::GetLastFrameCameraCacheView() const
{
    return LastFrameCameraCachePrivate.POV;
}

void APlayerCameraManager::FillCameraCache(const FMinimalViewInfo& NewInfo)
{
    // Backup last frame results.
    const float CurrentCacheTime = GetCameraCacheTime();
    const float CurrentGameTime = GetWorld()->TimeSeconds;
    if (CurrentCacheTime != CurrentGameTime)
    {
        SetLastFrameCameraCachePOV(GetCameraCacheView());
        SetLastFrameCameraCacheTime(CurrentCacheTime);
    }

    SetCameraCachePOV(NewInfo);
    SetCameraCacheTime(CurrentGameTime);
}

void APlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
    // Don't update outgoing viewtarget during an interpolation 
	if (PendingViewTarget.Target != nullptr && OutVT.Equal(ViewTarget))
	{
		return;
	}

	// Store previous POV, in case we need it later
	FMinimalViewInfo OrigPOV = OutVT.POV;

	// Reset the view target POV fully
	static const FMinimalViewInfo DefaultViewInfo;
	OutVT.POV = DefaultViewInfo;
	OutVT.POV.FOV = DefaultFOV;
    OutVT.POV.PostProcessBlendWeight = 1.0f;

	bool bDoNotApplyModifiers = false;

    //카메라가 없으면 액터 자기위치와 자기 로테이션 반영
    OutVT.POV.Location = OutVT.Target->GetActorLocation() + OutVT.Target->GetActorForwardVector() * 2;
    OutVT.POV.Rotation = OutVT.Target->GetActorRotation();
    
	if (UCameraComponent* CamComp = OutVT.Target->GetComponentByClass<UCameraComponent>())
	{
		// Viewing through a camera actor.
		CamComp->GetCameraView(DeltaTime, OutVT.POV);
	}
    
	ApplyCameraModifiers(DeltaTime, OutVT.POV);
}

/* A로부터 B로의 ViewTarget Blend 수행
 * 실제 언리얼 코드에선 사용하지 않음
 */
FMinimalViewInfo APlayerCameraManager::BlendViewTargets(const FTViewTarget& A, const FTViewTarget& B, float Alpha)
{
    FMinimalViewInfo POV;
    POV.Location = FMath::Lerp(A.POV.Location, B.POV.Location, Alpha);
    POV.FOV = (A.POV.FOV + Alpha * (B.POV.FOV - A.POV.FOV));

    FRotator DeltaAng = (B.POV.Rotation - A.POV.Rotation).GetNormalized();
    POV.Rotation = A.POV.Rotation + DeltaAng * Alpha;

    return POV;
}

void APlayerCameraManager::AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams)
{
    if (!NewTarget || NewTarget == VT.Target)
    {
        return;
    }
    
    // AActor* OldViewTarget = VT.Target;
    VT.Target = NewTarget;

    // Use default FOV and aspect ratio.
    VT.POV.FOV = DefaultFOV;

    // EndViewTarget시 행동을 해야하면 정의 후 부르기
    // if (OldViewTarget)
    // {
    //     OldViewTarget->EndViewTarget(PCOwner);
    // }

    // ViewTarget 세팅됐을때 행동을 해야하면 정의 후 부르기
    // VT.Target->BecomeViewTarget(PCOwner);

    PCOwner->ClientSetViewTarget(VT.Target, TransitionParams);
    
    // ViewTarget바뀔때 Delegate필요하면 호출
    // FGameDelegates::Get().GetViewTargetChangedDelegate().Broadcast(PCOwner, OldViewTarget, NewTarget);
}
