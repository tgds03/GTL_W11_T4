#include "ShaderRegisters.hlsl"

struct InstanceData
{
    float4 ParticleColor : TEXCOORD1;

    /** The instance to world transform of the particle. Translation vector is packed into W components. */
    float4 Transform1 : TEXCOORD2;
    float4 Transform2 : TEXCOORD3;
    float4 Transform3 : TEXCOORD4;

    /** The velocity of the particle, XYZ: direction, W: speed. */
    float4 Velocity : TEXCOORD5;
    float RelativeTime : RELATIVE_TIME;    
};

struct VS_INPUT_StaticMeshParticle
{
    // Slot 0 - Vertex Data
    VS_INPUT_StaticMesh MeshData;
    
    // Slot 1 - Instance Data
    InstanceData Instance;
};

struct PS_INPUT_StaticMeshParticle
{
    float4 Position : SV_POSITION;
};

PS_INPUT_StaticMeshParticle mainVS(VS_INPUT_StaticMeshParticle Input)
{
    PS_INPUT_StaticMeshParticle Output;

    float4x4 ModelMatrix = float4x4(
        float4(Input.Instance.Transform1.x, Input.Instance.Transform2.x, Input.Instance.Transform3.x, 0),
        float4(Input.Instance.Transform1.y, Input.Instance.Transform2.y, Input.Instance.Transform3.y, 0),
        float4(Input.Instance.Transform1.z, Input.Instance.Transform2.z, Input.Instance.Transform3.z, 0),
        float4(Input.Instance.Transform1.w, Input.Instance.Transform2.w, Input.Instance.Transform3.w, 1));
    

    Output.Position = float4(Input.MeshData.Position, 1.0);
    Output.Position = mul(Output.Position, ModelMatrix);
    //Output.WorldPosition = Output.Position.xyz;
    
    Output.Position = mul(Output.Position, ViewMatrix);
    Output.Position = mul(Output.Position, ProjectionMatrix);
    
    //Output.WorldNormal = mul(Input.Normal, (float3x3)InverseTransposedWorld);

    // Begin Tangent
    // float3 WorldTangent = mul(Input.Tangent.xyz, (float3x3)WorldMatrix);
    // WorldTangent = normalize(WorldTangent);
    // WorldTangent = normalize(WorldTangent - Output.WorldNormal * dot(Output.WorldNormal, WorldTangent));

    // Output.WorldTangent = float4(WorldTangent, Input.Tangent.w);
    // End Tangent
    
    // Output.UV = Input.UV;
    // Output.MaterialIndex = Input.MaterialIndex;
    // Output.Color = Input.Color;
    return Output;
}
