// triangle.hlsl
// what the fuck should i do
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Col : COLOR0;
};


VS_OUTPUT VS(float4 Pos : POSITION, float4 Col : COLOR)
{
    VS_OUTPUT ret;
    
    ret.Pos = Pos;
    ret.Col = Col;
    
    return ret;
}
