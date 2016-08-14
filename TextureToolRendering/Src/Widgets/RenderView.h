#pragma once
FORWARD_DECLARE_CLASS(Texture);
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

	// Returns the widget's WinID, which is actually a windows handle
	HWND                    GetWindowHandle() { return (HWND)winId(); }

	ID3D11RasterizerState*  GetRasterizerState() { return mRasterizerState; }

	// Renders the actual frame to the QT Widget.
	void                    Blit();

protected:

	// The render target view
	ID3D11RenderTargetView* mRenderTargetView;

	// The rasterizer state
	ID3D11RasterizerState*  mRasterizerState;

	// The swap chain
	IDXGISwapChain*			mSwapChain;

	// The sampler state
	ID3D11SamplerState*     mSamplerState;

	// Resize the swap chain and render target buffer
	void                    OnResized();

	// Catch resize events, to update the swapchain and rendertarget
	virtual void            resizeEvent(QResizeEvent* pEvent) override;

};

