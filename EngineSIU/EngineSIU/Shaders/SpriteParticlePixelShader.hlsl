Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

struct PS_INPUT_SpriteParticle
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
    float SubImageIndex : SUB_IMAGE_INDEX;             
    float SubImagesHorizontal : SUB_IMAGE_Horizontal;  
    float SubImagesVertical : SUB_IMAGE_Vertical;      
};

float4 mainPS(PS_INPUT_SpriteParticle Input) : SV_Target
{
    //return float4(1, 1, 1, 1);
    float2 SubUVScale = float2(1.0 / Input.SubImagesHorizontal, 1.0 / Input.SubImagesVertical);
    int index = (int)Input.SubImageIndex;
    int x = index % Input.SubImagesHorizontal;
    int y = index / Input.SubImagesHorizontal;

    float2 SubUVOffset = float2(x, y) * SubUVScale;
    
    float2 UV = Input.UV * SubUVScale + SubUVOffset;

    float4 Color = Texture.Sample(Sampler, UV);
    Color.xyzw *= Input.Color.xyzw;
    float threshold = 0.01f;
    
    if (max(max(Color.r, Color.g), Color.b) < threshold || Color.a < 0.1f)
    {
        discard;
    }
    // Color.a = .4f;
    return Color;
}
