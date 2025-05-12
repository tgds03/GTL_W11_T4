    #pragma once

    #include "UObject/Object.h"
    #include "UObject/ObjectMacros.h"
    #include "Engine/Source/Runtime/Launch/SkeletalDefine.h"
    #include "Engine/Source/Runtime/Core/Container/Array.h"
    #include "Engine/Source/Runtime/Engine/Classes/Components/MeshComponent.h"

    class USkeletalMesh;

    class USkinnedMeshComponent : public UMeshComponent 
    {
        DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

    public:
        USkinnedMeshComponent();
        ~USkinnedMeshComponent() = default;

        virtual UObject* Duplicate(UObject* InOuter) override;
        void GetProperties(TMap<FString, FString>& OutProperties) const override;
        void SetProperties(const TMap<FString, FString>& InProperties) override;

        virtual int CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const override;

    #pragma region Skeletal
        void SetSkeletalMesh(USkeletalMesh* InMesh);
        USkeletalMesh* GetSkeletalMesh() const { return SkeletalMesh; }
        void UpdateGlobalPose();
    #pragma endregion

        void SetselectedSubMeshIndex(const int& value) { selectedSubMeshIndex = value; }
        int GetselectedSubMeshIndex() const { return selectedSubMeshIndex; }

    protected:
        /** Skeletal mesh asset containing FSkeleton and source vertices */
        USkeletalMesh* SkeletalMesh;

        int selectedSubMeshIndex = -1;
    };
