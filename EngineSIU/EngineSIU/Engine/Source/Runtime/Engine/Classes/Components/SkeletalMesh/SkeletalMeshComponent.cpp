#include "SkeletalMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Engine/Source/Runtime/CoreUObject/UObject/ObjectFactory.h"

#include "Engine/Source/Runtime/Engine/Classes/Engine/Asset/SkeletalMeshAsset.h"

// 임시로 StaticMesh를 활용하면서 참고하게 된 코드 이후 제거 필요
#include "Engine/Source/Runtime/Engine/Classes/Components/Mesh/StaticMeshRenderData.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Asset/StaticMeshAsset.h"

// FBX 테스트를 위해 넣은 코드 이후 제거 필요
#include "Engine/Source/Runtime/Engine/Classes/Engine/FbxLoader.h"
#include "Animation/AnimSequence.h"

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

void USkeletalMeshComponent::GenerateSampleData()
{
    FResourceManager::CreateStaticMesh("Contents/Reference/Reference.obj");

    UStaticMesh* staticMesh = FResourceManager::GetStaticMesh(L"Contents/Reference/Reference.obj");
    if (!staticMesh) return;

    // 2) 새 SkeletalMesh 생성
    USkeletalMesh* skelMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);

    // 3) Z축을 따라 위치한 10개의 본 체인
    const int32 BoneCount = 10;
    TArray<FBone> Bones;
    Bones.Reserve(BoneCount);
    for (int32 i = 0; i < BoneCount; ++i)
    {
        FBone Bone;
        Bone.Name = FName(*FString::Printf(TEXT("Bone_%d"), i));
        Bone.ParentIndex = (i == 0 ? INDEX_NONE : i - 1);
        // 로컬 트랜스폼: Z축으로 0.1 위로 구성
        FBonePose localTransform;
        localTransform.Location.Z = 0.1f;
        Bone.LocalTransform = localTransform;
        Bones.Add(Bone);
    }
    //skelMesh->InitializeSkeleton(Bones);
    FSkeleton ParsedSkeleton(Bones);

    // 4) StaticMesh 정점 → SkeletalVertex 바인딩
    FStaticMeshRenderData* rd = staticMesh->GetRenderData();

    for (const FStaticMeshVertex& v : rd->Vertices)
    {
        FSkeletalMeshVertex sv;
        sv.X = v.X;
        sv.Y = v.Y;
        sv.Z = v.Z;
        int32 idx = FMath::Clamp(int32(v.Z / 0.2f), 0, BoneCount - 1);
        sv.BoneIndices[0] = idx;
        sv.BoneWeights[0] = 1.f;
        for (int j = 1; j < 4; ++j)
        {
            sv.BoneIndices[j] = INDEX_NONE;
            sv.BoneWeights[j] = 0.f;
        }
        skelMesh->GetRenderData()->Vertices.Add(sv);
    }

    // 5) 렌더 데이터 복제
    FStaticMeshRenderData* srcRD = staticMesh->GetRenderData();
    FSkeletalMeshRenderData* dstRD = new FSkeletalMeshRenderData();

    // 아래는 FSkeletalMeshRenderData가 FStaticMeshRenderData와 동일한 필드를 갖는다고 가정한 복제 예시입니다.
    dstRD->ObjectName = srcRD->ObjectName;
    dstRD->DisplayName = srcRD->DisplayName;
    dstRD->Materials = srcRD->Materials;
    dstRD->MaterialSubsets = srcRD->MaterialSubsets;
    dstRD->Skeleton = ParsedSkeleton;


    // 복제하긴 하지만 실제로 사용되는 건 SkeletalMesh의 SkeletalVertex를 사용해야함
    // 그렇게 해야만 Bone이 Vertex에 영향을 주게 됨
    
    // ■ Vertex 타입이 다르므로 각각 복제
    int32 NumVerts = srcRD->Vertices.Num();
    dstRD->Vertices.SetNum(NumVerts);
    for (int32 i = 0; i < NumVerts; ++i)
    {
        const FStaticMeshVertex& SrcV = srcRD->Vertices[i];
        FSkeletalMeshVertex& DstV = dstRD->Vertices[i];

        // 필드 단순 복사
        DstV.X = SrcV.X; DstV.Y = SrcV.Y; DstV.Z = SrcV.Z;
        DstV.R = SrcV.R; DstV.G = SrcV.G; DstV.B = SrcV.B; DstV.A = SrcV.A;
        DstV.NormalX = SrcV.NormalX; DstV.NormalY = SrcV.NormalY; DstV.NormalZ = SrcV.NormalZ;
        DstV.TangentX = SrcV.TangentX; DstV.TangentY = SrcV.TangentY;
        DstV.TangentZ = SrcV.TangentZ; DstV.TangentW = SrcV.TangentW;
        DstV.U = SrcV.U; DstV.V = SrcV.V;
        DstV.MaterialIndex = SrcV.MaterialIndex;
    }

    // 인덱스와 바운딩 박스 복사
    dstRD->Indices.SetNum(srcRD->Indices.Num());
    for (int32 i = 0; i < srcRD->Indices.Num(); ++i)
    {
        dstRD->Indices[i] = srcRD->Indices[i];
    }

    dstRD->BoundingBoxMin = srcRD->BoundingBoxMin;
    dstRD->BoundingBoxMax = srcRD->BoundingBoxMax;

    // 이제 복제본을 세팅
    skelMesh->SetData(dstRD);

    // 6) 이 컴포넌트에 세팅
    SetSkeletalMesh(skelMesh);

    // Bone 구조에 RenderData 가 영향을 받도록 UpdateAnimation 호출
    UpdateSkinnedPositions();

}

void USkeletalMeshComponent::TestSkeletalMesh()
{
    // 1) SkeletalMesh가 세팅되어 있는지 검사
    USkeletalMesh* SkelMesh = GetSkeletalMesh();
    if (!SkelMesh)
    {
        return;
    }

    // 2) 루트(0)를 제외한 모든 본에 대해 LocalTransform을 Y축으로 10도 기울이기
    const int32 BoneCount = SkelMesh->GetSkeleton()->BoneCount;
    for (int32 BoneIndex = 1; BoneIndex < BoneCount; ++BoneIndex)
    {
        FBonePose& localTransform = SkelMesh->GetSkeleton()->Bones[BoneIndex].LocalTransform;

        localTransform.Rotation = localTransform.Rotation * FQuat::CreateRotation(0, 0, 10);

        // 적용
        //SkelMesh->SetBoneLocalTransform(BoneIndex, localTransform);
    }

    // 3) 변경된 본 트랜스폼을 바탕으로 애니메이션 업데이트
    UpdateSkinnedPositions();
}

void USkeletalMeshComponent::TestFBXSkeletalMesh()
{
    // 1) FBX로부터 USkeletalMesh 생성
    FString FbxPath(TEXT("Assets/fbx/Twerk.fbx"));
    //FString FbxPath(TEXT("Contents/FbxTest/nathan3.fbx"));
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
        UpdateSkinnedPositions();
        return;
    }

    InitializeAnimInstance();
    // 2) SkeletalMeshComponent에 세팅
    SetSkeletalMesh(LoadedMesh);

    // 5) AnimInstance에 애니메이션 시퀀스 설정
    AnimInstance->SetAnimSequence(AnimSequence);


    UpdateSkinnedPositions();
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
    TArray<FTransform> BoneTransforms;
    AnimInstance->GetBoneTransforms(BoneTransforms);

    // 계산된 트랜스폼을 스켈레탈 메시에 적용 (예: 내부 함수)
    SkeletalMesh->SetBoneTransforms(BoneTransforms);

    // 스키닝된 버텍스 위치 계산
    SkelPosition.Empty();
    TArray<FSkeletalMeshVertex> SkelVertices = SkeletalMesh->GetRenderData()->Vertices;
    for (int i = 0; i < SkelVertices.Num(); i++)
    {
       // SkelPosition.Add(SkelVertices[i].GetSkinnedPosition(SkeletalMesh->GetSkeleton()));
    }
}
