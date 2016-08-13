#include "TT_PCH.h"

RenderView::RenderView(QWidget* pParent)
	: QWidget(pParent)
{
// Disable automatic clearing of the screen
	setUpdatesEnabled(false);
}

RenderView::~RenderView()
{
}

QPaintEngine* RenderView::paintEngine() const
{
	return nullptr; // Return a NULL paintEngine since we aren't using QT's paintEngine
}

// Override the paintEvent since it does not work correctly with DirectX
void RenderView::paintEvent(QPaintEvent *event)
{
}

// Override the nativeEvent because we want to catch WM_PAINT events, paintEvent won't work on its own
bool RenderView::nativeEvent(const QByteArray& pEventType, void* pMsg, long* pResult)
{
	Base::nativeEvent(pEventType, pMsg, pResult);

	MSG* message = static_cast<MSG*>(pMsg);
	if (message->message == WM_PAINT) // Catch WM_PAINT event
	{
		emit WMPaintRequested(); // Emit signal that the view needs to be redrawn
		return true; // Return true to tell QT that this signal is already handled
	}
	return false;
}

