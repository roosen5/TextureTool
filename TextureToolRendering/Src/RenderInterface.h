#pragma once
class RenderInterface : public QWidget
{
	Q_OBJECT
public:
	RenderInterface(RenderView* pRenderView);
	~RenderInterface();

	void                    Render2DTexture();
	// Initializes DirectX and sets up DXDevice variables.
	HRESULT                 InitializeDirectX();

	void                    Clear(const QColor& pClearColor);
	// Renders the actual frame to the QT Widget.
	void                    Blit();

protected:
	// Override the nativeEvent because we want to catch WM_PAINT events, QT's paintEvent doesn't work correctly with DirectX
	virtual bool            nativeEvent(const QByteArray &eventType, void *message, long *result) override;

	// The view where the render target is copied to
	RenderView*             mRenderView;

	// The render target view
	ID3D11RenderTargetView* mRenderTargetView;

	// The rasterizer state
	ID3D11RasterizerState*  mRasterizerState;

	// The swap chain
	IDXGISwapChain*			mSwapChain;

private slots:
	// Called when the WM_PAINT event is found in RenderView, redraws the widget
	void                    OnViewWMPaintRequested();

};

