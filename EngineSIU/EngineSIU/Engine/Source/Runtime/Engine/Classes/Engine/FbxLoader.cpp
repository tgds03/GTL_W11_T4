#include "FbxLoader.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Source/Runtime/Launch/SkeletalDefine.h"
#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include <filesystem>
#include "Engine/Source/Runtime/Core/Math/Matrix.h"

using namespace fbxsdk;

USkeletalMesh* FFbxLoader::LoadFBXSkeletalMeshAsset(const FString& filePathName)
{
    FWString key = filePathName.ToWideString();

    // 1) 캐시된 메시가 있으면 바로 반환
    if (USkeletalMesh** cached = SkeletalMeshMap.Find(key))
    {
        return *cached;
    }

    // 2) FBX SDK 초기화
    FbxManager* manager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);

    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(*filePathName, -1, manager->GetIOSettings()))
    {
        importer->Destroy();
        manager->Destroy();
        return nullptr;
    }

    FbxScene* scene = FbxScene::Create(manager, "SkeletalScene");
    importer->Import(scene);
    importer->Destroy();

    // 3) USkeletalMesh 생성
    USkeletalMesh* skelMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);

    // 4) 본 계층 파싱 (예: 재귀 순회로 FBone 배열 구성)
    {
        TArray<FBone> bones;
        bones.Reserve(64);

        std::function<void(FbxNode*, int)> recurse = [&](FbxNode* node, int parentIndex)
            {
                if (!node) return;
                auto* attr = node->GetNodeAttribute();
                if (attr && attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
                {
                    FBone b;
                    b.Name = FName(node->GetName());
                    b.ParentIndex = parentIndex;
                    const FbxAMatrix local = node->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);
                    b.LocalTransform = FMatrix::FromFbxMatrix(local);

                    int myIndex = bones.Num();
                    bones.Add(b);
                    if (parentIndex >= 0)
                        bones[parentIndex].Children.Add(myIndex);
                    parentIndex = myIndex;
                }
                for (int i = 0; i < node->GetChildCount(); ++i)
                    recurse(node->GetChild(i), parentIndex);
            };

        recurse(scene->GetRootNode(), -1);
        skelMesh->InitializeSkeleton(bones);
    }

    // 5) Mesh + Skin 데이터 파싱 (여러 메시 처리)
    {
        // (1) 렌더 데이터 준비
        FSkeletalMeshRenderData* rd = new FSkeletalMeshRenderData();
        rd->ObjectName = filePathName.ToWideString();
        FString displayName(rd->ObjectName.c_str());
        rd->DisplayName = displayName;
        rd->Materials.Empty();
        rd->MaterialSubsets.Empty();
        rd->Vertices.Empty();
        rd->Indices.Empty();

        // 재귀 함수: 노드 트리를 돌며 모든 FbxMesh 를 파싱
        std::function<void(FbxNode*)> recurse = [&](FbxNode* node)
            {
                if (!node) return;

                // 이 노드가 Mesh 를 가지고 있으면 처리
                if (FbxMesh* mesh = node->GetMesh())
                {
                    // (2) 이 메시 전용 시작 인덱스/버텍스 오프셋 저장
                    int indexStart = rd->Indices.Num();
                    int vertexStart = rd->Vertices.Num();

                    // (3) 머티리얼 슬롯 추가 (이 메시의 머티리얼만)
                    int matCount = node->GetMaterialCount();
                    int baseMatIndex = rd->Materials.Num();
                    for (int m = 0; m < matCount; ++m)
                    {
                        FObjMaterialInfo mi{};
                        mi.MaterialName = node->GetMaterial(m)->GetName();
                        rd->Materials.Add(mi);
                    }
                    if (matCount == 0)
                    {
                        // 머티리얼 없으면 기본 하나
                        FObjMaterialInfo mi{};
                        mi.MaterialName = TEXT("Default");
                        rd->Materials.Add(mi);
                        matCount = 1;
                    }

                    // (4) 스킨 웨이트 수집: control-point 당 (boneIndex, weight) 리스트
                    int cpCount = mesh->GetControlPointsCount();
                    std::vector<std::vector<std::pair<int, double>>> cpWeights(cpCount);
                    for (int si = 0; si < mesh->GetDeformerCount(FbxDeformer::eSkin); ++si)
                    {
                        FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(si, FbxDeformer::eSkin));
                        for (int ci = 0; ci < skin->GetClusterCount(); ++ci)
                        {
                            FbxCluster* cluster = skin->GetCluster(ci);
                            FbxNode* boneNode = cluster->GetLink();
                            if (!boneNode) continue;

                            // boneNode 이름 → skeleton 에서 인덱스 찾기
                            FName boneName = FName(boneNode->GetName());
                            int boneIndex = INDEX_NONE;
                            for (int b = 0; b < skelMesh->Skeleton.Bones.Num(); ++b)
                                if (skelMesh->Skeleton.Bones[b].Name == boneName)
                                {
                                    boneIndex = b;
                                    break;
                                }
                            if (boneIndex < 0) continue;

                            int* cpIdxArr = cluster->GetControlPointIndices();
                            double* wArr = cluster->GetControlPointWeights();
                            int     cnt = cluster->GetControlPointIndicesCount();
                            for (int k = 0; k < cnt; ++k)
                            {
                                int cpIdx = cpIdxArr[k];
                                double w = wArr[k];
                                if (cpIdx >= 0 && cpIdx < cpCount && w > 0.0)
                                    cpWeights[cpIdx].emplace_back(boneIndex, w);
                            }
                        }
                    }

                    // (5) 폴리곤(삼각형) 순회 → 정점·인덱스·스켈레탈 버텍스 생성
                    const FbxVector4* controlPoints = mesh->GetControlPoints();
                    auto* uvElem = mesh->GetElementUV();
                    const char* uvSetName = uvElem ? uvElem->GetName() : nullptr;

                    int polyCount = mesh->GetPolygonCount();
                    rd->Vertices.Reserve(rd->Vertices.Num() + polyCount * 3);
                    rd->Indices.Reserve(rd->Indices.Num() + polyCount * 3);
                    skelMesh->SourceVertices.Reserve(skelMesh->SourceVertices.Num() + polyCount * 3);

                    for (int p = 0; p < polyCount; ++p)
                    {
                        for (int v = 0; v < 3; ++v)
                        {
                            int cpIdx = mesh->GetPolygonVertex(p, v);
                            FbxVector4 pos;
                            pos = mesh->GetControlPointAt(cpIdx);

                            FbxVector4 nrm;
                            mesh->GetPolygonVertexNormal(p, v, nrm);

                            FbxVector2 uv;
                            bool unmapped = false;
                            if (uvSetName)
                                mesh->GetPolygonVertexUV(p, v, uvSetName, uv, unmapped);

                            // --- FSkeletalMeshVertex ---
                            FSkeletalMeshVertex outV{};
                            outV.X = (float)pos[0]; outV.Y = (float)pos[1]; outV.Z = (float)pos[2];
                            outV.R = outV.G = outV.B = outV.A = 1.f;
                            outV.NormalX = (float)nrm[0]; outV.NormalY = (float)nrm[1]; outV.NormalZ = (float)nrm[2];
                            outV.TangentX = outV.TangentY = outV.TangentZ = 0.f; outV.TangentW = 1.f;
                            outV.U = (float)uv[0];
                            outV.V = 1.f - (float)uv[1];
                            // 이 메시 전체에 걸쳐 머티리얼 인덱스 0 (추후 머티리얼 서브셋에서 구분)
                            outV.MaterialIndex = baseMatIndex;

                            int newVIdx = rd->Vertices.Num();
                            rd->Vertices.Add(outV);
                            rd->Indices.Add(newVIdx);

                            // --- FVertexSkeletal (CPU 스키닝용) ---
                            FVertexSkeletal srcV{};
                            srcV.Position = FVector(outV.X, outV.Y, outV.Z);

                            // cpWeights[cpIdx] 에서 상위 4개 추려 배분
                            auto& wlist = cpWeights[cpIdx];
                            std::sort(wlist.begin(), wlist.end(),
                                [](auto& A, auto& B) { return A.second > B.second; });
                            float totalW = 0.f;
                            int useN = FMath::Min((int)wlist.size(), 4);
                            for (int k = 0; k < useN; ++k)
                            {
                                srcV.BoneIndices[k] = wlist[k].first;
                                srcV.BoneWeights[k] = (float)wlist[k].second;
                                totalW += srcV.BoneWeights[k];
                            }
                            for (int k = useN; k < 4; ++k)
                            {
                                srcV.BoneIndices[k] = INDEX_NONE;
                                srcV.BoneWeights[k] = 0.f;
                            }
                            if (totalW > 0.f)
                                for (int k = 0; k < useN; ++k)
                                    srcV.BoneWeights[k] /= totalW;

                            skelMesh->SourceVertices.Add(srcV);
                        }
                    }

                    // (6) 이 메시 전용 머티리얼 서브셋 추가
                    FMaterialSubset subset;
                    subset.MaterialName = rd->Materials[baseMatIndex].MaterialName;
                    subset.MaterialIndex = baseMatIndex;
                    subset.IndexStart = indexStart;
                    subset.IndexCount = rd->Indices.Num() - indexStart;
                    rd->MaterialSubsets.Add(subset);
                }

                // 자식 노드들도 재귀 처리
                for (int i = 0; i < node->GetChildCount(); ++i)
                    recurse(node->GetChild(i));
            };

        recurse(scene->GetRootNode());

        // TODO (7) 바운딩 박스 계산은 이후에 추가 필요 

        // (8) 렌더 데이터 & 소스 정점 세팅
        skelMesh->SetData(rd);
    }

    return skelMesh;
}

USkeletalMesh* FFbxLoader::GetSkeletalMesh(const FWString& name)
{
    if (USkeletalMesh** found = SkeletalMeshMap.Find(name))
        return *found;
    return nullptr;
}
