// triangle.hlsl
// what the fuck should i do
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Col : COLOR0;
};


float4 PS(VS_OUTPUT input) : SV_Target
{
    return input.Col;
}
