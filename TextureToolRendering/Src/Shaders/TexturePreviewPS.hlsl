Texture2D Texture : register(t0);
sampler Sampler : register(s0) = sampler_state
{
	wrap = repeat;
};

struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 position : SV_POSITION;
};


float4 main( PixelShaderInput IN ) : SV_TARGET
{
	return (Texture.Sample(Sampler, IN.TexCoord));
}