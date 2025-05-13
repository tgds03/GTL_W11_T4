#include "SkeletalMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Engine/Source/Runtime/CoreUObject/UObject/ObjectFactory.h"

#include "Actors/Character/Character.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"

// FBX 테스트를 위해 넣은 코드 이후 제거 필요
#include <memory>

#include "Actors/Character/Pawn.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/FbxLoader.h"
#include "Animation/AnimSequence.h"
#include "Animation/UAnimationAsset.h" 

USkeletalMeshComponent::USkeletalMeshComponent()
    :USkinnedMeshComponent()
{
}

void USkeletalMeshComponent::InitializeAnimInstance(APawn* InOwner)
{
    OwnerPawn = InOwner;
    
    if (!AnimInstance)
    {
        AnimInstance = std::make_shared<UAnimInstance>();
        AnimInstance->Initialize(this, InOwner);
    }
}

void USkeletalMeshComponent::PlayAnimation(UAnimationAsset* NewAnimToPlay, bool bLooping)
{
    if (!AnimInstance)
    {
        return;
    }

    //SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SetAnimation(NewAnimToPlay);
    Play(bLooping);
}

void USkeletalMeshComponent::SetAnimation(UAnimationAsset* InAnimAsset)
{
}

void USkeletalMeshComponent::Play(bool bLooping)
{
}

void USkeletalMeshComponent::HandleAnimNotify(const FAnimNotifyEvent* Notify)
{
    if (AnimInstance)
    {
        // 임시 코드
        ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        OwnerCharacter->HandleAnimNotify(Notify);
    }
}

void USkeletalMeshComponent::GenerateSampleData()
{
}

void USkeletalMeshComponent::TestSkeletalDie()
{
    if (!OwnerPawn)
    {
        return; 
    }

    if (OwnerPawn->CurrentMovementMode == EDie)
    {
        OwnerPawn->CurrentMovementMode = EDancing;
    }else if (OwnerPawn->CurrentMovementMode == EDancing)
    {
        OwnerPawn->CurrentMovementMode = EDie;
    }
}

void USkeletalMeshComponent::TestFBXSkeletalMesh()
{
    // 1) FBX로부터 USkeletalMesh 생성

    FString FbxPath(TEXT("Contents/Fbx/Twerkbin.fbx"));
    
    USkeletalMesh* LoadedMesh = FResourceManager::LoadSkeletalMesh(FbxPath);
    if (!LoadedMesh)
    {
        UE_LOG(LogLevel::Warning, TEXT("FBX 로드 실패: %s"), *FbxPath);
        return;
    }

    UAnimSequence* AnimSequence = FResourceManager::LoadAnimationSequence(FbxPath);
    if (!AnimSequence)
    {
        UE_LOG(LogLevel::Warning, TEXT("애니메이션 로드 실패, 스켈레톤만 표시합니다."));
        UpdateGlobalPose();
        return;
    }

    // 2) SkeletalMeshComponent에 세팅
    SetSkeletalMesh(LoadedMesh);

    //////////////////////////////////
    FbxPath = TEXT("Contents/Fbx/Dying.fbx");
    UAnimSequence* AnimSequence2 = FResourceManager::LoadAnimationSequence(FbxPath);
    if (!AnimSequence2)
    {
        UE_LOG(LogLevel::Warning, TEXT("AnimSequence2 애니메이션 로드 실패."));
        return;
    }
    
    // AnimSequence->SetRateScale(-0.5f);

    // AS_Dance상태일땐 AnimSequence돌리라고 추가
    AnimInstance->AddAnimSequence(AS_Dance, AnimSequence);
    AnimInstance->AddAnimSequence(AS_Die, AnimSequence2);
    
    if (APawn* Actor = dynamic_cast<APawn*>(GetOwner()))
    {
        Actor->CurrentMovementMode = EDancing;
    }
    
    UpdateGlobalPose();
}

void USkeletalMeshComponent::PerformCPUSkinning()
{
    FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetRenderData();

    RenderData->CPUSkinnedVertices.SetNum(RenderData->Vertices.Num());
    RenderData->CPUSkinnedVertices = RenderData->Vertices; // 원본 복사
    
    // CPU 스키닝 수행 - 원본은 유지하고 복사본만 수정
    for (int32 i = 0; i < RenderData->Vertices.Num(); i++)
    {
        // 중요: 원본 Vertices에서 계산하지만 CPUSkinnedVertices에 저장
        FVector SkinnedPos = RenderData->Vertices[i].GetSkinnedPosition(SkeletalMesh->GetSkeletonPose());

        // 계산된 위치를 CPU 스키닝 결과 배열에 저장
        RenderData->CPUSkinnedVertices[i].X = SkinnedPos.X;
        RenderData->CPUSkinnedVertices[i].Y = SkinnedPos.Y;
        RenderData->CPUSkinnedVertices[i].Z = SkinnedPos.Z;
    }
}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    USkinnedMeshComponent::TickComponent(DeltaTime);

    //나중에 애니메이션 부를곳에서 부르기
    TickPose(DeltaTime);
}

void USkeletalMeshComponent::TickPose(float DeltaTime)
{
    USkinnedMeshComponent::TickPose(DeltaTime);

    TickAnimation(DeltaTime);
}

void USkeletalMeshComponent::TickAnimation(float DeltaTime)
{
    if (!SkeletalMesh || !AnimInstance)
    {
        return;
    }
    // AnimInstance 업데이트 (시간 진행 등)
    AnimInstance->Update(DeltaTime);

    if (!FEngineLoop::IsGPUSkinningEnabled()) 
    {
        PerformCPUSkinning();
    }

}

