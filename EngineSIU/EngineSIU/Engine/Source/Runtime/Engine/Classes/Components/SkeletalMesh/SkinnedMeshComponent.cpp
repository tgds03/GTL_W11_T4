#include "SkinnedMeshComponent.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/SkeletalMesh/SkeletalMesh.h"
#include "Engine/Source/Runtime/Engine/Classes/Asset/SkeletalMeshAsset.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

USkinnedMeshComponent::USkinnedMeshComponent()
    : Super(), SkeletalMesh(nullptr)
{

}

UObject* USkinnedMeshComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->SkeletalMesh = SkeletalMesh;
    NewComponent->selectedSubMeshIndex = selectedSubMeshIndex;

    return NewComponent;
}

void USkinnedMeshComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    //StaticMesh 경로 저장
    USkeletalMesh* CurrentMesh = GetSkeletalMesh();
    if (CurrentMesh != nullptr) {

        // 1. std::wstring 경로 얻기
        std::wstring PathWString = CurrentMesh->GetOjbectName(); // 이 함수가 std::wstring 반환 가정

        // 2. std::wstring을 FString으로 변환
        FString PathFString(PathWString.c_str()); // c_str()로 const wchar_t* 얻어서 FString 생성
        // PathFString = CurrentMesh->ConvertToRelativePathFromAssets(PathFString);

        FWString PathWString2 = PathFString.ToWideString();


        OutProperties.Add(TEXT("SkeletalMeshPath"), PathFString);
    }
    else
    {
        OutProperties.Add(TEXT("SkeletalMeshPath"), TEXT("None")); // 메시 없음 명시
    }
}

void USkinnedMeshComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;


    // --- StaticMesh 설정 ---
    TempStr = InProperties.Find(TEXT("SkeletalMeshPath"));
    if (TempStr) // 키가 존재하는지 확인
    {
        if (*TempStr != TEXT("None")) // 값이 "None"이 아닌지 확인
        {
            // 경로 문자열로 UStaticMesh 에셋 로드 시도

            if (USkeletalMesh* MeshToSet = FResourceManager::LoadSkeletalMesh(*TempStr))
            {
                SetSkeletalMesh(MeshToSet); // 성공 시 메시 설정
                UE_LOG(LogLevel::Display, TEXT("Set StaticMesh '%s' for %s"), **TempStr, *GetName());
            }
            else
            {
                // 로드 실패 시 경고 로그
                UE_LOG(LogLevel::Warning, TEXT("Could not load StaticMesh '%s' for %s"), **TempStr, *GetName());
                SetSkeletalMesh(nullptr); // 안전하게 nullptr로 설정
            }
        }
        else // 값이 "None"이면
        {
            SetSkeletalMesh(nullptr); // 명시적으로 메시 없음 설정
            UE_LOG(LogLevel::Display, TEXT("Set StaticMesh to None for %s"), *GetName());
        }
    }
    else // 키 자체가 없으면
    {
        // 키가 없는 경우 어떻게 처리할지 결정 (기본값 유지? nullptr 설정?)
        // 여기서는 기본값을 유지하거나, 안전하게 nullptr로 설정할 수 있습니다.
        // SetSkeletalMesh(nullptr); // 또는 아무것도 안 함
        UE_LOG(LogLevel::Display, TEXT("StaticMeshPath key not found for %s, mesh unchanged."), *GetName());
    }
}

int USkinnedMeshComponent::CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const
{
    if (!AABB.Intersect(InRayOrigin, InRayDirection, OutHitDistance))
    {
        return 0;
    }
    if (SkeletalMesh == nullptr)
    {
        return 0;
    }

    OutHitDistance = FLT_MAX;

    int IntersectionNum = 0;

    const int32 VertexNum = SkelPosition.Num();
    if (VertexNum == 0)
    {
        return 0;
    }
    
    const TArray<UINT>& Indices = SkeletalMesh->GetRenderData()->Indices;
    const int32 IndexNum = Indices.Num();
    const bool bHasIndices = (IndexNum > 0);

    int32 TriangleNum = bHasIndices ? (IndexNum / 3) : (VertexNum / 3);
    for (int32 i = 0; i < TriangleNum; i++)
    {
        int32 Idx0 = i * 3;
        int32 Idx1 = i * 3 + 1;
        int32 Idx2 = i * 3 + 2;

        if (bHasIndices)
        {
            Idx0 = Indices[Idx0];
            Idx1 = Indices[Idx1];
            Idx2 = Indices[Idx2];
        }

        // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
        FVector v0 = FVector(SkelPosition[Idx0].X, SkelPosition[Idx0].Y, SkelPosition[Idx0].Z);
        FVector v1 = FVector(SkelPosition[Idx1].X, SkelPosition[Idx1].Y, SkelPosition[Idx1].Z);
        FVector v2 = FVector(SkelPosition[Idx2].X, SkelPosition[Idx2].Y, SkelPosition[Idx2].Z);

        float HitDistance = FLT_MAX;
        if (IntersectRayTriangle(InRayOrigin, InRayDirection, v0, v1, v2, HitDistance))
        {
            OutHitDistance = FMath::Min(HitDistance, OutHitDistance);
            IntersectionNum++;
        }

    }
    return IntersectionNum;
}

void USkinnedMeshComponent::SetSkeletalMesh(USkeletalMesh* InMesh)
{
    SkeletalMesh = InMesh;
    if (SkeletalMesh == nullptr)
    {
        OverrideMaterials.SetNum(0);
        AABB = FBoundingBox(FVector::ZeroVector, FVector::ZeroVector);
    }
    else 
    {
        OverrideMaterials.SetNum(SkeletalMesh->GetMaterials().Num());
        AABB = FBoundingBox(SkeletalMesh->GetRenderData()->BoundingBoxMin, SkeletalMesh->GetRenderData()->BoundingBoxMax);
        UpdateAnimation();
    }
}

void USkinnedMeshComponent::UpdateAnimation()
{
    if (!SkeletalMesh)
    {
        return;
    }

    // 1. Update skeletal hierarchy global transforms
    SkeletalMesh->UpdateGlobalTransforms();

    SkelPosition.Empty();

    TArray<FSkeletalMeshVertex> SkelVertices = SkeletalMesh->GetRenderData()->Vertices;
    for (int i = 0; i < SkelVertices.Num(); i++) 
    {
        SkelPosition.Add(SkelVertices[i].GetSkinnedPosition(SkeletalMesh->GetSkeleton()));
    }
}




