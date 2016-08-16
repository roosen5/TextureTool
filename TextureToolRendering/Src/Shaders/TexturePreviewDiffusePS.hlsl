Texture2D Texture : register(t0);
sampler Sampler : register(s0) = sampler_state
{
	wrap = repeat;
};

cbuffer PreviewInfo : register(b0)
{
	int	forceMip;
	int renderChannels;
	bool normalize;
}


struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 position : SV_POSITION;
};

int ToFixedPoint(float4 pColor)
{
	int red   = (int)(min(1.0f, pColor.x) * 255.0f);
	int green = (int)(min(1.0f, pColor.y) * 255.0f);
	int blue  = (int)(min(1.0f, pColor.z) * 255.0f);
	int alpha = (int)(min(1.0f, pColor.w) * 255.0f);
	return (red << 24) + (green << 16) + (blue<<8) + alpha;
};

float4 ToRGBA(int pFixedPoint)
{
	float4 retVal = float4(0, 0, 0, 0);
	retVal.x = ((pFixedPoint >> 24) & 0xff) / 255.0f;
	retVal.y = ((pFixedPoint >> 16) & 0xff) / 255.0f;
	retVal.z = ((pFixedPoint >> 8) & 0xff) / 255.0f;
	retVal.w = ((pFixedPoint ) & 0xff) / 255.0f;
	return retVal;
};

float4 DoMask(float4 pColor, int pChannel)
{
	int colorFixedPoint = ToFixedPoint(pColor);
	colorFixedPoint = colorFixedPoint & pChannel;
	return ToRGBA(colorFixedPoint);
}

float4 main( PixelShaderInput IN ) : SV_TARGET
{
	float4 color=float4(0,0,0,0);
	if (forceMip == -1)
	{
		// Sample default
		color = Texture.Sample(Sampler, IN.TexCoord);
	}
	else
	{
		// Sample individual mipmap
		color = Texture.SampleLevel(Sampler, IN.TexCoord, forceMip);
	}

	if (renderChannels != 0xFFFFFFFF)
	{
		color = DoMask(color, renderChannels);

		if (normalize)
		{
			// To turn it white, used to preview single colors as white
			color = float4(length(color).xxx, 1.0f); 
		}

		// If the render channels does not include the alpha, force it to 1
		// Since we don't want to render blended in this case.
		if (ToRGBA(renderChannels).a == 0)
		{
			color.a = 1;
		}
	}

	return color;
}