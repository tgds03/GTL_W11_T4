#include "FbxLoader.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Source/Runtime/Launch/SkeletalDefine.h"
#include "Components/SkeletalMesh/SkeletalMeshComponent.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Asset/SkeletalMeshAsset.h"
#include "Engine/Source/Runtime/Engine/Classes/Components/SkeletalMesh/SkeletalMesh.h"
#include <filesystem>
#include "Engine/Source/Runtime/Core/Math/Matrix.h"
#include "Engine/Source/Runtime/Launch/Define.h"

using namespace fbxsdk;

USkeletalMesh* FFbxLoader::LoadFBXSkeletalMeshAsset(const FString& filePathName, FSkeletalMeshRenderData*& OutSkeletalMeshRenderData)
{
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
    TArray<FBone> bones;
    {
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
                    
                    FbxTime evalTime = FBXSDK_TIME_INFINITE;

                    const FbxAMatrix localT = node->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);

                    b.LocalTransform = FBonePose(FMatrix::FromFbxMatrix(localT));

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
        //skelMesh->InitializeSkeleton(bones);
    }
    FSkeleton ParsedSkeleton(bones);

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
        rd->Skeleton = ParsedSkeleton;

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

                    // FBX 파일 디렉터리 추출
                    std::wstring ws = filePathName.ToWideString();
                    size_t slashPos = ws.find_last_of(L"\\/");
                    FWString fbxDir = (slashPos != std::wstring::npos) ? ws.substr(0, slashPos + 1) : ws;

                    // (3) 머티리얼 슬롯 추가 (이 메시의 머티리얼만)
                    int matCount = node->GetMaterialCount();
                    int baseMatIndex = rd->Materials.Num();
                    for (int m = 0; m < matCount; ++m)
                    {
                        // FBX 머티리얼 객체
                        FbxSurfaceMaterial* fbxMat = node->GetMaterial(m);
                        FObjMaterialInfo mi{};
                        mi.MaterialName = fbxMat->GetName();

                        constexpr uint32 TexturesNum = static_cast<uint32>(EMaterialTextureSlots::MTS_MAX);
                        mi.TextureInfos.SetNum(TexturesNum);

                        uint32 SlotIdx = 0;

                        // Diffuse Color
                        FbxProperty DiffuseProp = fbxMat->FindProperty(FbxSurfaceMaterial::sDiffuse);
                        if (DiffuseProp.IsValid()) 
                        {
                            FbxDouble3 Diffuse = DiffuseProp.Get<FbxDouble3>();
                            mi.DiffuseColor = FVector(Diffuse[0], Diffuse[1], Diffuse[2]);
                        }

                        // Specular Color
                        FbxProperty SpecularProp = fbxMat->FindProperty(FbxSurfaceMaterial::sSpecular);
                        if (SpecularProp.IsValid())
                        {
                            FbxDouble3 Specular = SpecularProp.Get<FbxDouble3>();
                            mi.SpecularColor = FVector(Specular[0], Specular[1], Specular[2]);
                        }

                        // Shininess
                        FbxProperty ShininessProp = fbxMat->FindProperty(FbxSurfaceMaterial::sShininess);
                        if (ShininessProp.IsValid())
                        {
                            mi.Shininess = (float)ShininessProp.Get<FbxDouble>();
                        }


                        // Diffuse Texture
                        // Diffuse 채널에 연결된 파일 텍스처 검색
                        int DiffuseTexCount = DiffuseProp.GetSrcObjectCount<FbxFileTexture>();

                        if (DiffuseTexCount > 0)
                        {
                            // 첫 번째 텍스처만 사용 (필요에 따라 루프)
                            FbxFileTexture* fbxTex = DiffuseProp.GetSrcObject<FbxFileTexture>(0);
                            if (fbxTex)
                            {
                                FString texName = fbxTex->GetFileName(); 
                                FWString texturePath = texName.ToWideString();

                                if (CreateTextureFromFile(texturePath))
                                {
                                    SlotIdx = static_cast<uint32>(EMaterialTextureSlots::MTS_Diffuse);

                                    mi.TextureInfos[SlotIdx].TexturePath = texturePath;
                                    mi.TextureInfos[SlotIdx].bIsSRGB = true;
                                    mi.TextureFlag |= static_cast<uint16>(EMaterialTextureFlags::MTF_Diffuse);
                                }
                                else
                                {
                                    UE_LOG(LogLevel::Warning, TEXT("텍스처 로드 실패: %s"), *texName);
                                }
                            }
                        }

                        // Specular Texture
                        int SpecularTexCount = SpecularProp.GetSrcObjectCount<FbxTexture>();

                        if (SpecularTexCount > 0) 
                        {
                            // 첫 번째 텍스처만 사용 (필요에 따라 루프)
                            FbxFileTexture* fbxTex = SpecularProp.GetSrcObject<FbxFileTexture>(0);
                            if (fbxTex)
                            {
                                FString texName = fbxTex->GetFileName();
                                FWString texturePath = texName.ToWideString();

                                if (CreateTextureFromFile(texturePath))
                                {
                                    SlotIdx = static_cast<uint32>(EMaterialTextureSlots::MTS_Specular);

                                    mi.TextureInfos[SlotIdx].TexturePath = texturePath;
                                    mi.TextureInfos[SlotIdx].bIsSRGB = true;
                                    mi.TextureFlag |= static_cast<uint16>(EMaterialTextureFlags::MTF_Specular);
                                }
                                else
                                {
                                    UE_LOG(LogLevel::Warning, TEXT("텍스처 로드 실패: %s"), *texName);
                                }
                            }
                        }


                        // Normal Texture
                        FbxProperty BumpProp = fbxMat->FindProperty(FbxSurfaceMaterial::sNormalMap);
                        // Normal Map 이 없는 경우 Bump Map이라도 사용해보기
                        if (!BumpProp.IsValid()) 
                        {
                            BumpProp = fbxMat->FindProperty(FbxSurfaceMaterial::sBump);
                        }
                        int NormTexCount = BumpProp.GetSrcObjectCount<FbxTexture>();
                        if (NormTexCount > 0) 
                        {
                            // 첫 번째 텍스처만 사용 (필요에 따라 루프)
                            FbxFileTexture* fbxTex = BumpProp.GetSrcObject<FbxFileTexture>(0);
                            if (fbxTex)
                            {
                                FString texName = fbxTex->GetFileName(); 
                                FWString texturePath = texName.ToWideString();

                                if (CreateTextureFromFile(texturePath))
                                {
                                    SlotIdx = static_cast<uint32>(EMaterialTextureSlots::MTS_Normal);

                                    mi.TextureInfos[SlotIdx].TexturePath = texturePath;
                                    mi.TextureInfos[SlotIdx].bIsSRGB = true;
                                    mi.TextureFlag |= static_cast<uint16>(EMaterialTextureFlags::MTF_Normal);
                                }
                                else
                                {
                                    UE_LOG(LogLevel::Warning, TEXT("텍스처 로드 실패: %s"), *texName);
                                }
                            }
                        }



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
                            for (int b = 0; b < rd->Skeleton.BoneCount; ++b)
                                if (rd->Skeleton.Bones[b].Name == boneName)
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
                    FbxGeometryElementNormal* normalElem = mesh->GetElementNormal();
                    const char* uvSetName = uvElem ? uvElem->GetName() : nullptr;

                    int polyCount = mesh->GetPolygonCount();
                    // 최대 폴리곤 수 × 3으로 리저브해두면 충분합니다.
                    rd->Vertices.Reserve(rd->Vertices.Num() + polyCount * 3);
                    rd->Indices.Reserve(rd->Indices.Num() + polyCount * 3);

                    for (int p = 0; p < polyCount; ++p)
                    {
                        const int vertsInPoly = mesh->GetPolygonSize(p);

                        // 3각형
                        if (vertsInPoly == 3)
                        {
                            for (int v = 0; v < 3; ++v)
                            {
                                int cpIdx = mesh->GetPolygonVertex(p, v);
                                // 이하 기존 처리 로직 그대로…
                                FbxVector4 pos = mesh->GetControlPointAt(cpIdx);
                                FbxVector4 nrm; mesh->GetPolygonVertexNormal(p, v, nrm);
                                FbxVector2 uv; bool unmapped = false;
                                if (uvSetName)
                                    mesh->GetPolygonVertexUV(p, v, uvSetName, uv, unmapped);

                                // --- FSkeletalMeshVertex ---
                                FSkeletalMeshVertex outV{};
                                outV.X = (float)pos[0]; outV.Y = (float)pos[1]; outV.Z = (float)pos[2];
                                outV.R = outV.G = outV.B = outV.A = 1.f;
                                
                                FbxVector4 normal;
                                
                                if (normalElem)
                                {
                                    normal = GetNormalMappingVector(normalElem, mesh, p, v);
                                }
                                else 
                                {
                                    normal = nrm;
                                }
                                
                                
                                outV.NormalX = (float)normal[0]; outV.NormalY = (float)normal[1]; outV.NormalZ = (float)normal[2];
                                
                                outV.U = (float)uv[0]; outV.V = 1.f - (float)uv[1];
                                outV.MaterialIndex = baseMatIndex;


                                // --- 스키닝 GPU Shader에서 하기
                                auto& wlist = cpWeights[cpIdx];
                                std::sort(wlist.begin(), wlist.end(),
                                    [](auto& A, auto& B) { return A.second > B.second; });
                                float totalW = 0.f;
                                int useN = FMath::Min((int)wlist.size(), 4);
                                for (int k = 0; k < useN; ++k)
                                {
                                    outV.BoneIndices[k] = wlist[k].first;
                                    outV.BoneWeights[k] = (float)wlist[k].second;
                                    totalW += outV.BoneWeights[k];
                                }
                                for (int k = useN; k < 4; ++k)
                                {
                                    outV.BoneIndices[k] = INDEX_NONE;
                                    outV.BoneWeights[k] = 0.f;
                                }
                                if (totalW > 0.f)
                                    for (int k = 0; k < useN; ++k)
                                        outV.BoneWeights[k] /= totalW;

                                int newVIdx = rd->Vertices.Num();
                                rd->Vertices.Add(outV);
                                rd->Indices.Add(newVIdx);
                            }

                            int VerticeNum = rd->Vertices.Num();

                            if (VerticeNum >= 3) 
                            {
                                FSkeletalMeshVertex& Vertex0 = rd->Vertices[VerticeNum - 3];
                                FSkeletalMeshVertex& Vertex1 = rd->Vertices[VerticeNum - 2];
                                FSkeletalMeshVertex& Vertex2 = rd->Vertices[VerticeNum - 1];

                                CalculateTangent(Vertex0, Vertex1, Vertex2);
                                CalculateTangent(Vertex1, Vertex2, Vertex0);
                                CalculateTangent(Vertex2, Vertex0, Vertex1);
                            }
                        }
                        // 4각형이면 (0,2,1)과 (0,3,2) 두 개의 삼각형으로 분할
                        else if (vertsInPoly == 4)
                        {
                            static const int quadTriidx[2][3] = { {0,1,2}, {0,2,3} };
                            for (int tri = 0; tri < 2; ++tri)
                            {
                                for (int vi = 0; vi < 3; ++vi)
                                {
                                    int v = quadTriidx[tri][vi];
                                    int cpIdx = mesh->GetPolygonVertex(p, v);
                                    FbxVector4 pos = mesh->GetControlPointAt(cpIdx);
                                    FbxVector4 nrm; mesh->GetPolygonVertexNormal(p, v, nrm);
                                    FbxVector2 uv; bool unmapped = false;
                                    if (uvSetName)
                                        mesh->GetPolygonVertexUV(p, v, uvSetName, uv, unmapped);

                                    // --- FSkeletalMeshVertex ---
                                    FSkeletalMeshVertex outV{};
                                    outV.X = (float)pos[0]; outV.Y = (float)pos[1]; outV.Z = (float)pos[2];
                                    outV.R = outV.G = outV.B = outV.A = 1.f;
                                    
                                    FbxVector4 normal;

                                    if (normalElem)
                                    {
                                        normal = GetNormalMappingVector(normalElem, mesh, p, v);
                                    }
                                    else
                                    {
                                        normal = nrm;
                                    }


                                    outV.NormalX = (float)normal[0]; outV.NormalY = (float)normal[1]; outV.NormalZ = (float)normal[2];
                                    
                                    outV.U = (float)uv[0]; outV.V = 1.f - (float)uv[1];
                                    outV.MaterialIndex = baseMatIndex;

                                    
                                    // --- 스키닝 GPU Shader에서 하기
                                    auto& wlist = cpWeights[cpIdx];
                                    std::sort(wlist.begin(), wlist.end(),
                                        [](auto& A, auto& B) { return A.second > B.second; });
                                    float totalW = 0.f;
                                    int useN = FMath::Min((int)wlist.size(), 4);
                                    for (int k = 0; k < useN; ++k)
                                    {
                                        outV.BoneIndices[k] = wlist[k].first;
                                        outV.BoneWeights[k] = (float)wlist[k].second;
                                        totalW += outV.BoneWeights[k];
                                    }
                                    for (int k = useN; k < 4; ++k)
                                    {
                                        outV.BoneIndices[k] = INDEX_NONE;
                                        outV.BoneWeights[k] = 0.f;
                                    }
                                    if (totalW > 0.f)
                                        for (int k = 0; k < useN; ++k)
                                            outV.BoneWeights[k] /= totalW;

                                    int newVIdx = rd->Vertices.Num();
                                    rd->Vertices.Add(outV);
                                    rd->Indices.Add(newVIdx);
                                }

                                int VerticeNum = rd->Vertices.Num();

                                if (VerticeNum >= 3)
                                {
                                    FSkeletalMeshVertex& Vertex0 = rd->Vertices[VerticeNum - 3];
                                    FSkeletalMeshVertex& Vertex1 = rd->Vertices[VerticeNum - 2];
                                    FSkeletalMeshVertex& Vertex2 = rd->Vertices[VerticeNum - 1];

                                    CalculateTangent(Vertex0, Vertex1, Vertex2);
                                    CalculateTangent(Vertex1, Vertex2, Vertex0);
                                    CalculateTangent(Vertex2, Vertex0, Vertex1);
                                }
                            }
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

        // 바운딩 계산 작업
        ComputeBoundingBox(rd->Vertices, rd->BoundingBoxMin, rd->BoundingBoxMax);
        // (8) 렌더 데이터 & 소스 정점 세팅
        
        
        OutSkeletalMeshRenderData = rd;
        
        skelMesh->SetData(rd);
    }

    return skelMesh;
}

bool FFbxLoader::CreateTextureFromFile(const FWString& Filename, bool bIsSRGB)
{
    if (FEngineLoop::ResourceManager.GetTexture(Filename))
    {
        return true;
    }

    HRESULT hr = FEngineLoop::ResourceManager.LoadTextureFromFile(FEngineLoop::GraphicDevice.Device, Filename.c_str(), bIsSRGB);

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void FFbxLoader::CalculateTangent(FSkeletalMeshVertex& PivotVertex, const FSkeletalMeshVertex& Vertex1, const FSkeletalMeshVertex& Vertex2)
{
    const float s1 = Vertex1.U - PivotVertex.U;
    const float t1 = Vertex1.V - PivotVertex.V;
    const float s2 = Vertex2.U - PivotVertex.U;
    const float t2 = Vertex2.V - PivotVertex.V;
    const float E1x = Vertex1.X - PivotVertex.X;
    const float E1y = Vertex1.Y - PivotVertex.Y;
    const float E1z = Vertex1.Z - PivotVertex.Z;
    const float E2x = Vertex2.X - PivotVertex.X;
    const float E2y = Vertex2.Y - PivotVertex.Y;
    const float E2z = Vertex2.Z - PivotVertex.Z;

    const float Denominator = s1 * t2 - s2 * t1;
    FVector Tangent(1, 0, 0);
    FVector BiTangent(0, 1, 0);
    FVector Normal(PivotVertex.NormalX, PivotVertex.NormalY, PivotVertex.NormalZ);

    if (FMath::Abs(Denominator) > SMALL_NUMBER)
    {
        // 정상적인 계산 진행
        const float f = 1.f / Denominator;

        const float Tx = f * (t2 * E1x - t1 * E2x);
        const float Ty = f * (t2 * E1y - t1 * E2y);
        const float Tz = f * (t2 * E1z - t1 * E2z);
        Tangent = FVector(Tx, Ty, Tz).GetSafeNormal();

        const float Bx = f * (-s2 * E1x + s1 * E2x);
        const float By = f * (-s2 * E1y + s1 * E2y);
        const float Bz = f * (-s2 * E1z + s1 * E2z);
        BiTangent = FVector(Bx, By, Bz).GetSafeNormal();
    }
    else
    {
        // 대체 탄젠트 계산 방법
        // 방법 1: 다른 방향에서 탄젠트 계산 시도
        FVector Edge1(E1x, E1y, E1z);
        FVector Edge2(E2x, E2y, E2z);

        // 기하학적 접근: 두 에지 사이의 각도 이등분선 사용
        Tangent = (Edge1.GetSafeNormal() + Edge2.GetSafeNormal()).GetSafeNormal();

        // 만약 두 에지가 평행하거나 반대 방향이면 다른 방법 사용
        if (Tangent.IsNearlyZero())
        {
            // TODO: 기본 축 방향 중 하나 선택 (메시의 주 방향에 따라 선택)
            Tangent = FVector(1.0f, 0.0f, 0.0f);
        }
    }

    Tangent = (Tangent - Normal * FVector::DotProduct(Normal, Tangent)).GetSafeNormal();

    const float Sign = (FVector::DotProduct(FVector::CrossProduct(Normal, Tangent), BiTangent) < 0.f) ? -1.f : 1.f;

    PivotVertex.TangentX = Tangent.X;
    PivotVertex.TangentY = Tangent.Y;
    PivotVertex.TangentZ = Tangent.Z;
    PivotVertex.TangentW = Sign;
}


FbxVector4 FFbxLoader::GetNormalMappingVector(FbxGeometryElementNormal* normalElem, FbxMesh* mesh, int polygonIndex, int vertIndex)
{
    switch (normalElem->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
    {
        int cpIndex = mesh->GetPolygonVertex(polygonIndex, vertIndex);
        int dataIndex = 0;

        if (normalElem->GetReferenceMode() == FbxGeometryElement::eDirect)
        {
            dataIndex = cpIndex;
        }
        else
        {
            dataIndex = normalElem->GetIndexArray().GetAt(cpIndex);
        }

        return normalElem->GetDirectArray().GetAt(dataIndex);
    }
    break;

    case FbxGeometryElement::eByPolygonVertex:
    {
        int lIndex = mesh->GetPolygonVertexIndex(polygonIndex);
        int dataIndex = 0;

        if (normalElem->GetReferenceMode() == FbxGeometryElement::eDirect)
        {
            dataIndex = lIndex;
        }
        else // eIndexToDirect
        {
            dataIndex = normalElem->GetIndexArray().GetAt(lIndex);
        }

        return normalElem->GetDirectArray().GetAt(dataIndex);
    }
    break;

    case FbxGeometryElement::eAllSame:
    {
        return normalElem->GetDirectArray().GetAt(0);
    }
    break;

    default:
        // eByPolygon, eByEdge 등
        break;
    }

    return FbxVector4();
}

void FFbxLoader::ComputeBoundingBox(const TArray<FSkeletalMeshVertex>& InVertices, FVector& OutMinVector, FVector& OutMaxVector)
{
    FVector MinVector = { FLT_MAX, FLT_MAX, FLT_MAX };
    FVector MaxVector = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (int32 i = 0; i < InVertices.Num(); i++)
    {
        MinVector.X = std::min(MinVector.X, InVertices[i].X);
        MinVector.Y = std::min(MinVector.Y, InVertices[i].Y);
        MinVector.Z = std::min(MinVector.Z, InVertices[i].Z);

        MaxVector.X = std::max(MaxVector.X, InVertices[i].X);
        MaxVector.Y = std::max(MaxVector.Y, InVertices[i].Y);
        MaxVector.Z = std::max(MaxVector.Z, InVertices[i].Z);
    }

    OutMinVector = MinVector;
    OutMaxVector = MaxVector;
}

void FFbxLoader::CollectSkeletonNodesRecursive(FbxNode* Node, TArray<FbxNode*>& OutBoneNodes)
{
    if (!Node)
    {
        return;
    }

    // 노드가 스켈레톤인지 확인
    bool bIsBone = Node->GetNodeAttribute() &&
        (Node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton);

    // 메시를 가진 노드도 본으로 취급 (스키닝된 메시의 경우)
    if (!bIsBone && Node->GetMesh() && Node->GetMesh()->GetDeformerCount(FbxDeformer::eSkin) > 0)
    {
        bIsBone = true;
    }

    if (bIsBone)
    {
        OutBoneNodes.Add(Node);
    }

    // 자식 노드 처리
    for (int i = 0; i < Node->GetChildCount(); i++)
    {
        CollectSkeletonNodesRecursive(Node->GetChild(i), OutBoneNodes);
    }
}


bool FFbxLoader::LoadFBXAnimationAsset(const FString& filePathName, UAnimDataModel* OutAnimDataModel)
{

    if (filePathName.IsEmpty() || !OutAnimDataModel)
    {
        return false;
    }

    // FBX 씬 로드 - 기존 LoadFBXSkeletalMeshAsset 코드 재활용
    FbxManager* manager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);
    FbxImporter* importer = FbxImporter::Create(manager, "");

    if (!importer->Initialize(*filePathName, -1, manager->GetIOSettings()))
    {
        importer->Destroy();
        manager->Destroy();
        return false;
    }

    FbxScene* scene = FbxScene::Create(manager, "AnimationScene");
    importer->Import(scene);
    importer->Destroy();


    // 애니메이션 스택(Take) 가져오기
    FbxAnimStack* AnimStack = scene->GetCurrentAnimationStack();
    if (!AnimStack)
    {
        UE_LOG(LogLevel::Warning, TEXT("애니메이션 스택이 없습니다."));
        scene->Destroy();
        manager->Destroy();
        return false;
    }

    // 애니메이션 시간 범위 가져오기
    FbxTimeSpan TimeSpan = AnimStack->GetLocalTimeSpan();
    FbxTime Start = TimeSpan.GetStart();
    FbxTime End = TimeSpan.GetStop();

    // 프레임 레이트 설정
    FbxGlobalSettings& GlobalSettings = scene->GetGlobalSettings();
    FbxTime::EMode TimeMode = GlobalSettings.GetTimeMode();
    double FrameRate = FbxTime::GetFrameRate(TimeMode);

    // 애니메이션 정보 설정
    OutAnimDataModel->SetPlayLength((float)(End - Start).GetSecondDouble());
    OutAnimDataModel->SetFrameRate(FFrameRate(FrameRate, 1.0));
    OutAnimDataModel->SetNumberOfFrames((int32)(End - Start).GetFrameCount(TimeMode));
    OutAnimDataModel->SetNumberOfKeys(OutAnimDataModel->GetNumberOfKeys());

    // 본 목록 수집
    TArray<FbxNode*> BoneNodes;
    CollectSkeletonNodesRecursive(scene->GetRootNode(), BoneNodes);

    if (BoneNodes.Num() == 0)
    {
        UE_LOG(LogLevel::Warning, TEXT("스켈레톤 노드가 없습니다: %s"), *filePathName);
        scene->Destroy();
        manager->Destroy();
        return false;
    }

    // 각 본에 대한 애니메이션 트랙 추출
    for (FbxNode* BoneNode : BoneNodes)
    {
        FBoneAnimationTrack BoneTrack;
        BoneTrack.Name = FName(BoneNode->GetName());

        // 키프레임 데이터 공간 할당
        BoneTrack.InternalTrackData.PosKeys.SetNum(OutAnimDataModel->GetNumberOfFrames());
        BoneTrack.InternalTrackData.RotKeys.SetNum(OutAnimDataModel->GetNumberOfFrames());
        BoneTrack.InternalTrackData.ScaleKeys.SetNum(OutAnimDataModel->GetNumberOfFrames());

        // 프레임 간격 계산
        FbxTime FrameTime;
        FrameTime.SetSecondDouble(1.0 / FrameRate); // 한 프레임당 시간 계산

        // 각 프레임에 대한 트랜스폼 추출
        for (int32 FrameIndex = 0; FrameIndex < OutAnimDataModel->GetNumberOfFrames(); ++FrameIndex)
        {
            // 현재 프레임 시간 계산
            FbxTime CurrentTime = Start + FrameTime * FrameIndex;

            // 글로벌 트랜스폼 가져오기
            FbxAMatrix GlobalTransform = BoneNode->EvaluateGlobalTransform(CurrentTime);

            // 로컬 트랜스폼 계산 (부모가 있는 경우)
            FbxAMatrix LocalTransform;
            if (BoneNode->GetParent())
            {
                FbxAMatrix ParentGlobal = BoneNode->GetParent()->EvaluateGlobalTransform(CurrentTime);
                LocalTransform = ParentGlobal.Inverse() * GlobalTransform;
            }
            else
            {
                LocalTransform = GlobalTransform;
            }

            // FBX 행렬에서 위치, 회전, 스케일 추출
            FbxVector4 Translation = LocalTransform.GetT();
            FbxQuaternion Rotation = LocalTransform.GetQ();
            FbxVector4 Scale = LocalTransform.GetS();

            // 언리얼 엔진 형식으로 변환 (Y와 Z축 변환 포함)
            FVector Position(
                (float)Translation[0],
                (float)Translation[2],  // Y와 Z 교환
                (float)Translation[1]
            );

            FQuat RotQuat(
                (float)Rotation[0],
                (float)Rotation[2],  // Y와 Z 교환
                (float)Rotation[1],
                (float)Rotation[3]
            );

            FVector ScaleVec(
                (float)Scale[0],
                (float)Scale[2],  // Y와 Z 교환
                (float)Scale[1]
            );

            // 키프레임 데이터 설정
            BoneTrack.InternalTrackData.PosKeys[FrameIndex] = Position;
            BoneTrack.InternalTrackData.RotKeys[FrameIndex] = RotQuat;
            BoneTrack.InternalTrackData.ScaleKeys[FrameIndex] = ScaleVec;
        }

        // 애니메이션 트랙 추가
        OutAnimDataModel->AddBoneTrack(BoneTrack);
    }

    // FBX 객체 정리
    scene->Destroy();
    manager->Destroy();

    return OutAnimDataModel->GetBoneAnimationTracks().Num() > 0;
}
