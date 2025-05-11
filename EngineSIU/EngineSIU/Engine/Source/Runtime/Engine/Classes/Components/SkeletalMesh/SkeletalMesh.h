#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"
#include "Components/Material/Material.h"
//#include "Engine/Source/Runtime/Engine/Classes/Engine/Asset/SkeletalMeshAsset.h"
#include "Engine/Source/Runtime/Launch/SkeletalDefine.h"

struct FSkeletalMeshRenderData;

class USkeletalMesh : public UObject
{
    DECLARE_CLASS(USkeletalMesh, UObject)

public:
    USkeletalMesh() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    USkeletalMesh* DuplicateSkeletalMesh();

    FSkeletalMeshRenderData* GetRenderData() const { return RenderData; }
    const TArray<FStaticMaterial*>& GetMaterials() const { return materials; }
    uint32 GetMaterialIndex(FName MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*>& OutMaterial) const;

    /** Set the local transform for a specific bone by index */
    void SetBoneLocalTransform(int boneIndex, const FBonePose& localTransform);

    /** Recompute global transforms for all bones */
    void UpdateGlobalTransforms();
      
    FSkeleton* GetSkeleton() const;
    FSkeletonPose* GetSkeletonPose() { return &SkeletonPose; }
    TArray<FBonePose>& GetLocalTransforms() { return SkeletonPose.LocalTransforms; }
    TArray<FMatrix>& GetGlobalTransforms() { return SkeletonPose.GlobalTransforms; }

    void SetData(FSkeletalMeshRenderData* InRenderData, FSkeletonPose InSkeletonPose);

public:
    //ObjectName은 경로까지 포함
    FWString GetOjbectName() const;
    void SetBoneTransforms(const TArray<FTransform>& InBoneTransforms);

private:
    FSkeletalMeshRenderData* RenderData = nullptr;
    FSkeletonPose SkeletonPose;
    TArray<FStaticMaterial*> materials;
};
