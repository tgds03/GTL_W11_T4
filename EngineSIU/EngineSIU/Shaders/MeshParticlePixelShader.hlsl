#include "ShaderRegisters.hlsl"

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

// cbuffer DiffuseMultiplierConstants : register(b6)
// {
//     float DiffuseMultiplier;
//     float3 DiffuseOverrideColor;
// }

struct PS_INPUT_MeshParticle
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
    float3 WorldNormal : TEXCOORD1;
    float4 WorldTangent : TEXCOORD2;
    float3 WorldPosition : TEXCOORD3;
    nointerpolation uint MaterialIndex : MATERIAL_INDEX;
};

float4 mainPS(PS_INPUT_MeshParticle Input) : SV_Target
{
    //return float4(1, 0, 0, 1);

    // Diffuse
    float3 DiffuseColor = Material.DiffuseColor;
    if (Material.TextureFlag & TEXTURE_FLAG_DIFFUSE)
    {
        float4 DiffuseColor4 = MaterialTextures[TEXTURE_SLOT_DIFFUSE].Sample(MaterialSamplers[TEXTURE_SLOT_DIFFUSE], Input.UV);
        if (DiffuseColor4.a < 0.1f)
        {
            discard;
        }
        DiffuseColor = DiffuseColor4.rgb;
        // W08
        // if (DiffuseMultiplier > 0.1)
        // {
        //     float3 OtherColor = MaterialTextures[TEXTURE_SLOT_AMBIENT].Sample(MaterialSamplers[TEXTURE_SLOT_AMBIENT], Input.UV).rgb;
        //     DiffuseColor = lerp(DiffuseColor, OtherColor, DiffuseMultiplier * DiffuseMultiplier);
        // }
        //
    }
    
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

//     if (IsLit)
//     {
//         float3 LitColor = float3(0, 0, 0);
// #ifdef LIGHTING_MODEL_GOURAUD
//         LitColor = Input.Color.rgb;
// #elif defined(LIGHTING_MODEL_PBR)
//         LitColor = Lighting(Input.WorldPosition, WorldNormal, ViewWorldLocation, DiffuseColor, Metallic, Roughness);
// #else
//         LitColor = Lighting(Input.WorldPosition, WorldNormal, ViewWorldLocation, DiffuseColor, SpecularColor, Shininess, FlatTileIndex);
// #endif
//         LitColor += EmissiveColor * 5.f; // 5는 임의의 값
//         FinalColor = float4(LitColor, 1);
//     }
//     else
    {
        FinalColor = float4(DiffuseColor * Input.Color.xyz, 1);
    }

    return FinalColor;
}
