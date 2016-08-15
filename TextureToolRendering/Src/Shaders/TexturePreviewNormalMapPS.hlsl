Texture2D Texture : register(t0);
sampler Sampler : register(s0) = sampler_state
{
	wrap = repeat;
};

cbuffer PreviewInfo : register(b0)
{
	int	forceMip;
}


struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 position : SV_POSITION;
};


float4 main( PixelShaderInput IN ) : SV_TARGET
{
	float4 color;
	if (forceMip == -1)
		color = Texture.Sample(Sampler, IN.TexCoord);
	else
		color = Texture.SampleLevel(Sampler, IN.TexCoord, forceMip);

	color.b = sqrt(1-saturate(dot(color.rg, color.rg)));
	color.a = 1;
	return color;
}