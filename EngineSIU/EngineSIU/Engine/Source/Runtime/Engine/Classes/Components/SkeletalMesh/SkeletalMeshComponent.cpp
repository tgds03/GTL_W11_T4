#include "SkeletalMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Engine/Source/Runtime/CoreUObject/UObject/ObjectFactory.h"

#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"

// 임시로 StaticMesh를 활용하면서 참고하게 된 코드 이후 제거 필요
#include "Engine/Source/Runtime/Engine/Classes/Components/Mesh/StaticMeshRenderData.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/StaticMeshAsset.h"

// FBX 테스트를 위해 넣은 코드 이후 제거 필요
#include "Engine/Source/Runtime/Engine/Classes/Engine/FbxLoader.h"

USkeletalMeshComponent::USkeletalMeshComponent()
    :USkinnedMeshComponent()
{
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
    UpdateAnimation();
}

void USkeletalMeshComponent::TestFBXSkeletalMesh()
{
    // 1) FBX로부터 USkeletalMesh 생성

    FString FbxPath(TEXT("Assets/FBX/XBot.fbx"));
    //FString FbxPath(TEXT("Assets/FBX/Macarena.fbx"));
    //FString FbxPath(TEXT("Assets/FBX/nathan3.fbx"));

    USkeletalMesh* LoadedMesh = FResourceManager::LoadSkeletalMesh(FbxPath);
    if (!LoadedMesh)
    {
        UE_LOG(LogLevel::Warning, TEXT("FBX 로드 실패: %s"), *FbxPath);
        return;
    }

    // 2) SkeletalMeshComponent에 세팅
    SetSkeletalMesh(LoadedMesh);

    UpdateAnimation();
}
