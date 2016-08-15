#pragma once
class TextureConverter
{
public:
	TextureConverter();
	~TextureConverter();
	//
	void     SetOriginalTexture(Texture* pTexture) { mTexture = pTexture; }
	HRESULT Convert(Texture*& pOutTexture);

private:
	Texture* mTexture;
};

