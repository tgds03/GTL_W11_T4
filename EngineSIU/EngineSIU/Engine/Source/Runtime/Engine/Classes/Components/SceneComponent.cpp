#include "Components/SceneComponent.h"
#include "Math/Rotator.h"
#include "Math/JungleMath.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include "Engine/HitResult.h""
#include "GameFramework/Actor.h"

USceneComponent::USceneComponent()
    : RelativeLocation(FVector(0.f, 0.f, 0.f))
    , RelativeRotation(FVector(0.f, 0.f, 0.f))
    , RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
}

UObject* USceneComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->RelativeLocation = RelativeLocation;
    NewComponent->RelativeRotation = RelativeRotation;
    NewComponent->RelativeScale3D = RelativeScale3D;

    return NewComponent;
}

void USceneComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("RelativeLocation"), *RelativeLocation.ToString());
    OutProperties.Add(TEXT("RelativeRotation"), *RelativeRotation.ToString());
    OutProperties.Add(TEXT("RelativeScale3D"), *RelativeScale3D.ToString());

    USceneComponent* ParentComp = GetAttachParent();
    if (ParentComp != nullptr)
    {
        OutProperties.Add(TEXT("AttachParentID"), ParentComp->GetName());
    }
    else
    {
        OutProperties.Add(TEXT("AttachParentID"), "nullptr");
    }
}

void USceneComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("RelativeLocation"));
    if (TempStr)
    {
        RelativeLocation.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("RelativeRotation"));
    if (TempStr)
    {
        RelativeRotation.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("RelativeScale3D"));
    if (TempStr)
    {
        RelativeScale3D.InitFromString(*TempStr);
    }
}

void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void USceneComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}


int USceneComponent::CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const
{
    return 0;
}

void USceneComponent::DestroyComponent(bool bPromoteChildren)
{
    TArray<USceneComponent*> ChildrenCopy = AttachChildren;
    for (auto& Child : ChildrenCopy)
    {
        if (Child == nullptr)
        {
            continue;
        }

        if (bPromoteChildren)
        {
            Child->DestroyComponent(bPromoteChildren);
        }
        else
        {
            AActor* Owner = GetOwner();
            if (AttachParent)
            {
                Child->DetachFromComponent(this);
                // 자식 컴포넌트들을 부모에 어태치
                Child->SetupAttachment(AttachParent);
            }
            else if (Owner != nullptr)
            {
                if (Owner->GetRootComponent())
                {
                    Child->DetachFromComponent(this);
                    // 부모가 nullptr인 경우 Owner의 Root에라도 어태치
                    Child->SetupAttachment(Owner->GetRootComponent());
                }
                else
                {
                    // 루트 컴포넌트도 없는 경우, 아무거나 하나를 루트로 지정해줌
                    Owner->SetRootComponent(Child);       
                }
            }
        }
    }

    AttachChildren.Empty();

    if (AttachParent)
    {
        DetachFromComponent(AttachParent);
    }

    Super::DestroyComponent(bPromoteChildren);
}

FVector USceneComponent::GetForwardVector() const
{
    FVector Forward = FVector::ForwardVector;
    Forward = JungleMath::FVectorRotate(Forward, GetWorldRotation());
    return Forward;
}

FVector USceneComponent::GetRightVector() const
{
    FVector Right = FVector::RightVector;
    Right = JungleMath::FVectorRotate(Right, GetWorldRotation());
    return Right;
}

FVector USceneComponent::GetUpVector() const
{
    FVector Up = FVector::UpVector;
    Up = JungleMath::FVectorRotate(Up, GetWorldRotation());
    return Up;
}


void USceneComponent::AddLocation(const FVector& InAddValue)
{
    RelativeLocation = RelativeLocation + InAddValue;

}

void USceneComponent::AddRotation(const FRotator& InAddValue)
{
    RelativeRotation = RelativeRotation + InAddValue;
    RelativeRotation.Normalize();
}

void USceneComponent::AddScale(const FVector& InAddValue)
{
    RelativeScale3D = RelativeScale3D + InAddValue;
}

void USceneComponent::AttachToComponent(USceneComponent* InParent)
{
    // 기존 부모와 연결을 끊기
    if (AttachParent)
    {
        AttachParent->AttachChildren.Remove(this);
    }

    // InParent도 nullptr이면 부모를 nullptr로 설정
    if (InParent == nullptr)
    {
        AttachParent = nullptr;
        return;
    }


    // 새로운 부모 설정
    AttachParent = InParent;

    // 부모의 자식 리스트에 추가
    if (!InParent->AttachChildren.Contains(this))
    {
        InParent->AttachChildren.Add(this);
    }
}

void USceneComponent::SetWorldLocation(const FVector& InLocation)
{
    // TODO: 코드 최적화 방법 생각하기
    FMatrix NewRelativeMatrix = FMatrix::CreateTranslationMatrix(InLocation);
    if (AttachParent)
    {
        FMatrix ParentMatrix = AttachParent->GetWorldMatrix().GetMatrixWithoutScale();
        NewRelativeMatrix = NewRelativeMatrix * FMatrix::Inverse(ParentMatrix);
    }
    FVector NewRelativeLocation = NewRelativeMatrix.GetTranslationVector();
    RelativeLocation = NewRelativeLocation;
}

void USceneComponent::SetWorldRotation(const FRotator& InRotation)
{
    SetWorldRotation(InRotation.ToQuaternion());
}

void USceneComponent::SetWorldRotation(const FQuat& InQuat)
{
    // TODO: 코드 최적화 방법 생각하기
    FMatrix NewRelativeMatrix = InQuat.ToMatrix();
    if (AttachParent)
    {
        FMatrix ParentMatrix = AttachParent->GetWorldMatrix().GetMatrixWithoutScale();
        NewRelativeMatrix = NewRelativeMatrix * FMatrix::Inverse(ParentMatrix);
    }
    FQuat NewRelativeRotation = FQuat(NewRelativeMatrix);
    RelativeRotation = FRotator(NewRelativeRotation);
    RelativeRotation.Normalize();   
}

void USceneComponent::SetWorldScale3D(const FVector& InScale)
{
    // TODO: 코드 최적화 방법 생각하기
    FMatrix NewRelativeMatrix = FMatrix::CreateScaleMatrix(InScale.X, InScale.Y, InScale.Z);
    if (AttachParent)
    {
        FMatrix ParentMatrix = FMatrix::GetScaleMatrix(AttachParent->RelativeScale3D);
        NewRelativeMatrix = NewRelativeMatrix * FMatrix::Inverse(ParentMatrix);
    }
    FVector NewRelativeScale = NewRelativeMatrix.GetScaleVector();
    RelativeScale3D = NewRelativeScale;
}

FVector USceneComponent::GetWorldLocation() const
{
    return GetWorldMatrix().GetTranslationVector();
}

FRotator USceneComponent::GetWorldRotation() const
{
    FMatrix WorldMatrix = GetWorldMatrix().GetMatrixWithoutScale();
    FQuat Quat = WorldMatrix.ToQuat();
    return FRotator(Quat);
}

FVector USceneComponent::GetWorldScale3D() const
{
    return GetWorldMatrix().GetScaleVector();
}

FMatrix USceneComponent::GetScaleMatrix() const
{
    return FMatrix::GetScaleMatrix(RelativeScale3D);
}

FMatrix USceneComponent::GetRotationMatrix() const
{
    return FMatrix::GetRotationMatrix(RelativeRotation);
}

FMatrix USceneComponent::GetTranslationMatrix() const
{
    return FMatrix::GetTranslationMatrix(RelativeLocation);
}

FMatrix USceneComponent::GetWorldMatrix() const
{
    FMatrix ScaleMat = GetScaleMatrix();
    FMatrix RotationMat = GetRotationMatrix();
    FMatrix TranslationMat = GetTranslationMatrix();

    FMatrix RTMat = RotationMat * TranslationMat;

    USceneComponent* Parent = AttachParent;
    while (Parent)
    {
        FMatrix ParentScaleMat = Parent->GetScaleMatrix();
        FMatrix ParentRotationMat = Parent->GetRotationMatrix();
        FMatrix ParentTranslationMat = Parent->GetTranslationMatrix();
        
        ScaleMat = ScaleMat * ParentScaleMat;
        
        FMatrix ParentRTMat = ParentRotationMat * ParentTranslationMat;
        RTMat = RTMat * ParentRTMat;

        Parent = Parent->AttachParent;
    }
    return ScaleMat * RTMat;
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    if (
        InParent != AttachParent                                  // 설정하려는 Parent가 기존의 Parent와 다르거나
        && InParent != this                                       // InParent가 본인이 아니고
        && InParent != nullptr                                    // InParent가 유효한 포인터 이며
        && (
            AttachParent == nullptr                               // AttachParent도 유효하며
            || !AttachParent->AttachChildren.Contains(this)  // 한번이라도 SetupAttachment가 호출된적이 없는 경우
        ) 
    )
    {
        AttachParent = InParent;

        // TODO: .AddUnique의 실행 위치를 RegisterComponent로 바꾸거나 해야할 듯
        InParent->AttachChildren.AddUnique(this);
    }
}

void USceneComponent::DetachFromComponent(USceneComponent* Target)
{
    // TODO: Detachment Rule 필요

    if (!Target || !Target->AttachChildren.Contains(this))
    {
        return;
    }

    Target->AttachChildren.Remove(this);
}

void USceneComponent::SetRelativeRotation(const FRotator& InRotation)
{
    SetRelativeRotation(InRotation.GetNormalized().ToQuaternion());
}

void USceneComponent::SetRelativeRotation(const FQuat& InQuat)
{
    FQuat NormalizedQuat = InQuat.GetSafeNormal();

    RelativeRotation = NormalizedQuat.Rotator();
    RelativeRotation.Normalize();
}

FTransform USceneComponent::GetComponentTransform() const
{
    FQuat Rotation = GetWorldRotation().ToQuaternion();
    FVector Location = GetWorldLocation();
    FVector Scale = GetRelativeScale3D();

    return FTransform(Rotation, Location, Scale);
}

void USceneComponent::UpdateOverlaps(const TArray<FOverlapInfo>* PendingOverlaps, bool bDoNotifies, const TArray<const FOverlapInfo>* OverlapsAtEndLocation)
{
    UpdateOverlapsImpl(PendingOverlaps, bDoNotifies, OverlapsAtEndLocation);
}

bool USceneComponent::MoveComponent(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit)
{
    return MoveComponentImpl(Delta, NewRotation, bSweep, OutHit);
}

bool USceneComponent::MoveComponent(const FVector& Delta, const FRotator& NewRotation, bool bSweep, FHitResult* OutHit)
{
    return MoveComponentImpl(Delta, NewRotation.ToQuaternion(), bSweep, OutHit);
}

void USceneComponent::UpdateOverlapsImpl(const TArray<FOverlapInfo>* PendingOverlaps, bool bDoNotifies, const TArray<const FOverlapInfo>* OverlapsAtEndLocation)
{
    TArray<USceneComponent*> AttachedChildren(AttachChildren);
    for (USceneComponent* ChildComponent : AttachedChildren)
    {
        if (ChildComponent)
        {
            ChildComponent->UpdateOverlaps(PendingOverlaps);
        }
    }
}

bool USceneComponent::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit)
{
    if (!this)
    {
        if (OutHit)
        {
            *OutHit = FHitResult();
        }
        return false;
    }

    if (OutHit)
    {
        *OutHit = FHitResult(1.f);
    }

    if (!Delta.IsNearlyZero() || !NewRotation.Equals(GetWorldRotation().ToQuaternion()))
    {
        SetWorldLocation(GetWorldLocation() + Delta);
        SetWorldRotation(NewRotation);

        UpdateOverlaps();
    }

    return true;
}

bool USceneComponent::IsUsingAbsoluteRotation() const
{
    return bAbsoluteRotation; 
}

void USceneComponent::SetUsingAbsoluteRotation(const bool bInAbsoluteRotation)
{
    bAbsoluteRotation = bInAbsoluteRotation;
}
