struct AppData
{
	float3 position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float2 TexCoord : TEXCOORD0;
	float4 position : SV_POSITION;
};

VertexShaderOutput main( AppData IN )
{
    VertexShaderOutput OUT;

	OUT.position = float4(IN.position, 1.0f);
	OUT.TexCoord = IN.TexCoord;

    return OUT;
}