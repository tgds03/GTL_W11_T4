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

    /**
     * Assign the skeletal mesh asset to this component.
     */
    void SetSkeletalMesh(USkeletalMesh* InMesh);

    USkeletalMesh* GetSkeletalMesh() { return SkeletalMesh; }

    /**
     * Recalculate bone transforms and skin vertices. Call once per frame.
     */
    void UpdateAnimation();

    virtual void TickComponent(float DeltaTime) override;

    /**
     * Retrieve the most recently skinned vertex positions.
     */
    const TArray<FVector>& GetSkinnedVertices() const;

    void SetselectedSubMeshIndex(const int& value) { selectedSubMeshIndex = value; }
    int GetselectedSubMeshIndex() const { return selectedSubMeshIndex; }

private:
    /** Skeletal mesh asset containing FSkeleton and source vertices */
    USkeletalMesh* SkeletalMesh;

    /** Cached skinned positions after UpdateAnimation */
    TArray<FVector> SkinnedPositions;

    int selectedSubMeshIndex = -1;
};
