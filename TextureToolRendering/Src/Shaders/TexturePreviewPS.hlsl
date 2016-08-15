Texture2D Texture : register(t0);
sampler Sampler : register(s0) = sampler_state
{
	wrap = repeat;
};

cbuffer PreviewInfo : register(b0)
{
	int	forceMip;
	int renderChannels;
}


struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 position : SV_POSITION;
};

float4 main( PixelShaderInput IN ) : SV_TARGET
{
	if (forceMip == -1)
	{
		float4 color = Texture.Sample(Sampler, IN.TexCoord);
		return color;
	}
	return (Texture.SampleLevel(Sampler, IN.TexCoord, forceMip));
}