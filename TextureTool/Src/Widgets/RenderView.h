#pragma once

FORWARD_DECLARE_CLASS(Texture);
FORWARD_DECLARE_CLASS(Shader);
FORWARD_DECLARE_CLASS(Model);

#define MIPMAP_AUTO -1

class RenderView : public QWidget
{
	Q_OBJECT
	typedef QWidget Base;
public:

	RenderView(QWidget* pWidget=nullptr);
	~RenderView();

	// Create a plane and render a 2D texture on it
	void                    Render2DTexture(const Texture* pTexture);

	// Initializes DirectX and sets up DXDevice variables.
	HRESULT                 Initialize();

	// Clear the render target
	void                    Clear(const QColor& pClearColor);

	// Set the mipmap to pMipmap
	void                    SetForcedMipmap(int pMipmap) { mForcedMipmap = pMipmap; }

	int                     GetForcedMipmap() { return mForcedMipmap; }

	// Sets the renderchannels integer, to be converted to RGBA inside the HLSL shader.
	// The color asks as a mask
	void                    SetRenderChannels(int pRenderChannels) { mRenderChannels = pRenderChannels; }

	int                     GetRenderChannels() { return mRenderChannels; }

	// Returns the widget's WinID, which is actually a windows handle
	HWND                    GetWindowHandle() { return (HWND)winId(); }

	ID3D11RasterizerState*  GetRasterizerState() { return mRasterizerState; }


	// Sets the current pixel shader of the rendering model
	void                    SetCurrentPixelShader(const wchar_t* pShaderName);

	// Retrieves the pixel shader as const, because we don't want it changed outside via the RenderView
	const Shader*           GetCurrentPixelShader();


	// Sets the current vertex shader of the rendering model
	void                    SetCurrentVertexShader(const wchar_t* pShaderName);

	// Retrieves the vertex shader as const, because we don't want it changed outside via the RenderView
	const Shader*           GetCurrentVertexShader();

	// Renders the actual frame to the QT Widget.
	void                    Blit();

signals:
	void                    Resized();
protected:

	// The render target view
	ID3D11RenderTargetView* mRenderTargetView;

	// The rasterizer state
	ID3D11RasterizerState*  mRasterizerState;

	// The swap chain
	IDXGISwapChain*			mSwapChain;

	// The sampler state
	ID3D11SamplerState*     mSamplerState;

	// The blend state, defining whether we render alpha or not
	ID3D11BlendState*       mBlendState;

	// Resize the swap chain and render target buffer
	void                    OnResized();

	// The channels that will be rendered, acts as a mask
	int                     mRenderChannels;

	// Whether or not the color is the value of the length of the other colors
	bool                    mGrayScaleOutput;

	// To render individual mipmap, if its set to MIPMAP_AUTO(-1) it will be auto
	int                     mForcedMipmap;

	// Catch resize events, to update the swapchain and rendertarget
	virtual void            resizeEvent(QResizeEvent* pEvent) override;

	// The model (That is just a plane) where the texture is drawn upon
	Model*                  mTexturePreviewModel;


};

