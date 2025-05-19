struct PS_INPUT_StaticMeshParticle
{
    float4 Position : SV_POSITION;
};

float4 mainPS(PS_INPUT_StaticMeshParticle Input) : SV_Target
{
    return float4(1, 0, 0, 1);
}
