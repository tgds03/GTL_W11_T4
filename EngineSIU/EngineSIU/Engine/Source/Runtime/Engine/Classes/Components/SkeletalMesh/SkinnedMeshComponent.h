#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Source/Runtime/Launch/SkeletalDefine.h"
#include "Engine/Source/Runtime/Core/Container/Array.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/SkeletalMesh/SkeletalMesh.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/MeshComponent.h"

class USkinnedMeshComponent : public UMeshComponent 
{
    DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

public:
    USkinnedMeshComponent();
    ~USkinnedMeshComponent() = default;

    //TODO 아래 요소 채우기
    /*virtual UObject* Duplicate(UObject* InOuter) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;*/

#pragma region Skeletal
    void SetSkeletalMesh(USkeletalMesh* InMesh);
    USkeletalMesh* GetSkeletalMesh() { return SkeletalMesh; }
    void UpdateAnimation();
    const TArray<FVector>& GetSkinnedVertices() const;
#pragma endregion

    void SetselectedSubMeshIndex(const int& value) { selectedSubMeshIndex = value; }
    int GetselectedSubMeshIndex() const { return selectedSubMeshIndex; }

protected:
    /** Skeletal mesh asset containing FSkeleton and source vertices */
    USkeletalMesh* SkeletalMesh;

    /** Cached skinned positions after UpdateAnimation */
    TArray<FVector> SkinnedPositions;

    int selectedSubMeshIndex = -1;
};
