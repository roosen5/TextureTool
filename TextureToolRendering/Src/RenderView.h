#ifndef RENDERVIEW_H
#define RENDERVIEW_H

FORWARD_DECLARE_CLASS(RenderInterface);
class RenderView : public QWidget
{
	Q_OBJECT

	typedef QWidget Base;
public:
	RenderView(QWidget* pParent);
	~RenderView();

	// Returns the widget's WinID, which is actually a windows handle
	HWND                    GetWindowHandle() { return (HWND)winId(); }

signals:
	// The signal that will be emitted when the View wants to be re-drawn
	void                    WMPaintRequested();

protected:
	// Override the paintEngine and return null inside it
	virtual QPaintEngine*	paintEngine() const override;

	// Override the paintEvent since it does not work correctly with DirectX
	virtual void            paintEvent(QPaintEvent *event) override;

	// Override the nativeEvent because we want to catch WM_PAINT events, paintEvent won't work on its own
	virtual bool            nativeEvent(const QByteArray &eventType, void *message, long *result) override;
};

#endif // RENDERVIEW_H
