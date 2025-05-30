#include "SkeletalMesh.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"
#include "Engine/FObjLoader.h"

#include "UObject/Casts.h"
#include "Engine/Source/Runtime/CoreUObject/UObject/ObjectFactory.h"

UObject* USkeletalMesh::Duplicate(UObject* InOuter)
{
    ThisClass* NewSkeletalMesh = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewSkeletalMesh->RenderData = this->RenderData;
    NewSkeletalMesh->materials.Reserve(this->materials.Num());

    NewSkeletalMesh->SkeletonPose = this->SkeletonPose.Duplicate();

    for (const auto& M : this->materials)
    {
        NewSkeletalMesh->materials.Emplace(M);
    }

    return NewSkeletalMesh;
}

USkeletalMesh* USkeletalMesh::DuplicateSkeletalMesh()
{
    ThisClass* NewSkeletalMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);

    NewSkeletalMesh->RenderData = this->RenderData;
    NewSkeletalMesh->materials.Reserve(this->materials.Num());

    NewSkeletalMesh->SkeletonPose = this->SkeletonPose.Duplicate();

    for (const auto& M : this->materials)
    {
        NewSkeletalMesh->materials.Emplace(M);
    }

    return NewSkeletalMesh;
}

uint32 USkeletalMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Num(); materialIndex++) {
        if (materials[materialIndex]->MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void USkeletalMesh::GetUsedMaterials(TArray<UMaterial*>& OutMaterial) const
{
    for (const FStaticMaterial* Material : materials)
    {
        OutMaterial.Emplace(Material->Material);
    }
}

FSkeleton* USkeletalMesh::GetSkeleton() const
{
    return SkeletonPose.Skeleton;
}

void USkeletalMesh::SetData(FSkeletalMeshRenderData* InRenderData, FSkeletonPose InSkeletonPose)
{
    RenderData = InRenderData;
    SkeletonPose = InSkeletonPose;

    materials.Empty();

    for (int materialIndex = 0; materialIndex < RenderData->Materials.Num(); materialIndex++)
    {
        FStaticMaterial* newMaterialSlot = new FStaticMaterial();

        UMaterial* newMaterial = FResourceManager::CreateMaterial(RenderData->Materials[materialIndex]);

        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->MaterialSlotName = RenderData->Materials[materialIndex].MaterialName;

        materials.Add(newMaterialSlot);
    }
}

void USkeletalMesh::SetBoneLocalTransform(int boneIndex, const FBonePose& localTransform)
{
    if (boneIndex >= 0 && boneIndex < SkeletonPose.Skeleton->BoneCount)
    {
        SkeletonPose.LocalTransforms[boneIndex] = localTransform;
    }

    // GlobalTransform 갱신은 Render 찍기 전에만 딱 한번 하도록 하기!
}

void USkeletalMesh::UpdateGlobalTransforms()
{
    SkeletonPose.ComputeGlobalTransforms();
}

FWString USkeletalMesh::GetObjectName() const
{
    return RenderData->ObjectName;
}

void USkeletalMesh::SetBoneLocalTransforms(const TArray<FBonePose>& InBoneTransforms)
{
    if (!RenderData || !SkeletonPose.Skeleton->BoneCount)
        return;

    // 스켈레톤의 본 개수와 입력 트랜스폼 배열의 크기 확인
    const int32 BoneCount = SkeletonPose.Skeleton->BoneCount;
    if (InBoneTransforms.Num() != BoneCount)
    {
        // 개수가 맞지 않으면 오류 메시지 출력 및 반환
        UE_LOG(LogLevel::Error, TEXT("SetBoneTransforms: 본 트랜스폼 개수 불일치 (스켈레톤: %d, 입력: %d)"),
            BoneCount, InBoneTransforms.Num());
        return;
    }

     //모든 본의 로컬 트랜스폼 설정
    for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
    {
        // 개별 본 로컬 트랜스폼 설정
        SetBoneLocalTransform(BoneIndex, InBoneTransforms[BoneIndex]);
    }

    // 글로벌 트랜스폼 업데이트
    UpdateGlobalTransforms();
}

USkeletalMesh* USkeletalMesh::DeepDuplicateSkeletalMesh()
{
    // 새 USkeletalMesh 객체 생성
    USkeletalMesh* NewMesh = new USkeletalMesh();

    // 1. RenderData 복사
    if (this->RenderData)
    {
        // 새 RenderData 할당 및 복사
        NewMesh->RenderData = new FSkeletalMeshRenderData(*this->RenderData);
    }

    // 2. SkeletonPose 복사
    NewMesh->SkeletonPose = this->SkeletonPose;

    // LocalTransforms 깊은 복사 (필요시)
    NewMesh->SkeletonPose.LocalTransforms.Empty();
    for (const FBonePose& BonePose : this->SkeletonPose.LocalTransforms)
    {
        NewMesh->SkeletonPose.LocalTransforms.Add(BonePose);
    }

    // GlobalTransforms 깊은 복사
    NewMesh->SkeletonPose.GlobalTransforms.Empty();
    for (const FMatrix& Matrix : this->SkeletonPose.GlobalTransforms)
    {
        NewMesh->SkeletonPose.GlobalTransforms.Add(Matrix);
    }

    // 3. Materials 복사
    NewMesh->materials.Empty();
    for (FStaticMaterial* Material : this->materials)
    {
        if (Material)
        {
            // 메터리얼 포인터 복사 (메터리얼 자체는 공유해도 됨)
            NewMesh->materials.Add(Material);
        }
    }


    return NewMesh;
}
