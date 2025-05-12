#include "SkeletalMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Engine/Source/Runtime/CoreUObject/UObject/ObjectFactory.h"

#include "Actors/Character/Character.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"

// 임시로 StaticMesh를 활용하면서 참고하게 된 코드 이후 제거 필요
#include "Engine/Source/Runtime/Engine/Classes/Components/Mesh/StaticMeshRenderData.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/StaticMeshAsset.h"

// FBX 테스트를 위해 넣은 코드 이후 제거 필요
#include "Engine/Source/Runtime/Engine/Classes/Engine/FbxLoader.h"
#include "Animation/AnimSequence.h"
#include "Animation/UAnimationAsset.h" 

USkeletalMeshComponent::USkeletalMeshComponent()
    :USkinnedMeshComponent()
{
}

void USkeletalMeshComponent::InitializeAnimInstance()
{
    if (!AnimInstance)
    {
        AnimInstance = std::make_shared<UAnimInstance>();
        AnimInstance->Initialize(this);
    }
}

void USkeletalMeshComponent::UpdateAnimation(float DeltaTime)
{
    if (!SkeletalMesh || !AnimInstance)
    {
        return;
    }

    // AnimInstance 업데이트 (시간 진행 등)
    AnimInstance->Update(DeltaTime);

    // 현재 애니메이션 프레임의 본 트랜스폼 계산
    //TArray<FBonePose> BoneTransforms;
    //AnimInstance->GetBoneTransforms(BoneTransforms);

    // 계산된 트랜스폼을 스켈레탈 메시에 적용 (예: 내부 함수)
    //SkeletalMesh->SetBoneTransforms(BoneTransforms);

    if (!FEngineLoop::IsGPUSkinningEnabled()) 
    {
        PerformCPUSkinning();
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

void USkeletalMeshComponent::TestSkeletalMesh()
{
    if (!SkeletalMesh)
    {
        return; 
    }

    const int32 BoneCount = SkeletalMesh->GetSkeleton()->BoneCount;
    TArray<FBonePose>& LocalTransforms = SkeletalMesh->GetLocalTransforms();

    for (int32 BoneIndex = 1; BoneIndex < BoneCount; ++BoneIndex)
    {
        FBonePose& localTransform = LocalTransforms[BoneIndex];

        // 2) 루트(0)를 제외한 모든 본에 대해 LocalTransform을 Y축으로 10도 기울이기
        localTransform.Rotation = localTransform.Rotation * FQuat::CreateRotation(0, 0, 10);
    }

    // 3) 변경된 본 트랜스폼을 바탕으로 애니메이션 업데이트
    UpdateGlobalPose();
}

void USkeletalMeshComponent::TestFBXSkeletalMesh()
{
    FString FbxPath(TEXT("Contents/FBX/Twerkbin.fbx"));
    
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

    InitializeAnimInstance();

    // 2) SkeletalMeshComponent에 세팅
    SetSkeletalMesh(LoadedMesh);

    // 5) AnimInstance에 애니메이션 시퀀스 설정
    AnimInstance->SetAnimSequence(AnimSequence);

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

