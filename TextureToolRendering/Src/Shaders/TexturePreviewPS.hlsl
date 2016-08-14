
struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 position : SV_POSITION;
};


float4 main( PixelShaderInput IN ) : SV_TARGET
{
	return float4(IN.TexCoord, 0, 1);
}