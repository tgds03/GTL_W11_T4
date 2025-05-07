#include "StaticMeshVertexShader.hlsl"

// Weight 계산 외에는 StaticMeshVertexShader과 다른 것이 없으므로
// Weight 계산을 진행하고 StaticMeshVertexShader의 mainVS로 넘겨서 처리

#define MAX_BONE_NUM 256

cbuffer BoneWeightConstants : register(b2)
{
    row_major matrix BoneTransform[MAX_BONE_NUM];   // InvGlobal * GlobalTransform 한 값
    // Shader에서 InvGlobal * GlobalTransform을 안하는 이유는 Bone 단위로 한번만 해도 되는 구조이기 때문임
}

PS_INPUT_StaticMesh mainVS(VS_INPUT_SkeletalMesh Input)
{
    // Weight 계산을 진행해서 나온 Position을 기반으로 StaticMesh mainVS 실행시켜버리기
    float4 WeightedPosition = float4(0, 0, 0, 1);

    if (Input.BoneIndices.x >= 0)
    {
        WeightedPosition += Input.BoneWeights.x * mul(BoneTransform[Input.BoneIndices.x], float4(Input.Position, 1.0f));
    }
    if (Input.BoneIndices.y >= 0)
    {
        WeightedPosition += Input.BoneWeights.y * mul(BoneTransform[Input.BoneIndices.y], float4(Input.Position, 1.0f));
    }
    if (Input.BoneIndices.z >= 0)
    {
        WeightedPosition += Input.BoneWeights.z * mul(BoneTransform[Input.BoneIndices.z], float4(Input.Position, 1.0f));
    }
    if (Input.BoneIndices.w >= 0)
    {
        WeightedPosition += Input.BoneWeights.w * mul(BoneTransform[Input.BoneIndices.w], float4(Input.Position, 1.0f));
    }
    
    
    VS_INPUT_StaticMesh StaticMeshVertex;
    StaticMeshVertex.Position = WeightedPosition.xyz;
    StaticMeshVertex.Color = Input.Color;
    StaticMeshVertex.Normal = Input.Normal;
    StaticMeshVertex.Tangent = Input.Tangent;
    StaticMeshVertex.UV = Input.UV;
    StaticMeshVertex.MaterialIndex = Input.MaterialIndex;

    return mainVS(StaticMeshVertex);
}
