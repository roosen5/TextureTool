#pragma once
class RenderView : public QWidget
{
	Q_OBJECT
	typedef QWidget Base;
public:

	RenderView();
	~RenderView();

	void                    Render2DTexture();
	// Initializes DirectX and sets up DXDevice variables.
	HRESULT                 InitializeDirectX();

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

	// Resize the swap chain and render target buffer
	void                    OnResized();

	virtual void            resizeEvent(QResizeEvent* pEvent) override;

};

