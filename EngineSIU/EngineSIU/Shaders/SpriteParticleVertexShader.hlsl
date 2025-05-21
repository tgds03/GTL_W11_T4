#include "ShaderRegisters.hlsl"

struct VS_INPUT_Billboard
{
    float3 Position : POSITION0;
    float2 UV : TEXCOORD0;
};

struct InstanceData
{
    /** The position of the particle. */
    float3 Position : POSITION1;
    /** The relative time of the particle. */
    float RelativeTime : RELATIVE_TIME;
    /** The previous position of the particle. */
    float3	OldPosition : POSITION2;
    /** Value that remains constant over the lifetime of a particle. */
    float ParticleId : PARTICLE_ID;
    /** The size of the particle. */
    float2 Size : TEXCOORD1;
    /** The rotation of the particle. */
    float Rotation : ROTATION;
    /** The sub-image index for the particle. */
    float SubImageIndex : SUB_IMAGE_INDEX;
    float SubImagesHorizontal : SUB_IMAGE_Horizontal;
    float SubImagesVertical : SUB_IMAGE_Vertical;
    /** The color of the particle. */
    float4 Color : TEXCOORD2;
};

struct VS_INPUT_SpriteParticle
{
    // Slot 0 - Vertex Data
    VS_INPUT_Billboard VertexData;
    
    // Slot 1 - Instance Data
    InstanceData Instance;
};

struct PS_INPUT_SpriteParticle
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;

    float SubImageIndex : SUB_IMAGE_INDEX;             
    float SubImagesHorizontal : SUB_IMAGE_Horizontal;  
    float SubImagesVertical : SUB_IMAGE_Vertical;      
};

PS_INPUT_SpriteParticle mainVS(VS_INPUT_SpriteParticle Input)
{
    PS_INPUT_SpriteParticle Output;

    float CosR = cos(Input.Instance.Rotation);
    float SinR = sin(Input.Instance.Rotation);

    float2 Rotated = float2(
        Input.VertexData.Position.x * CosR - Input.VertexData.Position.y * SinR,
        Input.VertexData.Position.x * SinR + Input.VertexData.Position.y * CosR
    );

    float3 CameraRight = InvViewMatrix[0].xyz;
    float3 CameraUp = InvViewMatrix[1].xyz;
    float3 CameraForward = InvViewMatrix[2].xyz;
    
    // Billboard axis-aligned 위치 계산
    float3 worldPos = Input.Instance.Position + Rotated.x * CameraRight + Rotated.y * CameraUp;
    Output.Position = float4(worldPos, 1.0);
    Output.Position = mul(Output.Position, ViewMatrix);
    Output.Position = mul(Output.Position, ProjectionMatrix);



    //Output.UV = Input.VertexData.UV * uvScale + uvOffset;
    Output.UV = Input.VertexData.UV;
    Output.Color = Input.Instance.Color;
    Output.SubImageIndex = Input.Instance.SubImageIndex;
    Output.SubImagesHorizontal = Input.Instance.SubImagesHorizontal;
    Output.SubImagesVertical = Input.Instance.SubImagesVertical;
    // Input.Instance.SubImageIndex

    return Output;
}
