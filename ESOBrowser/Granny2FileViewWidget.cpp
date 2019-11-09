#include "Granny2FileViewWidget.h"
#include "ModelViewerWidget.h"
#include "ESOBrowserMainWindow.h"

Granny2FileViewWidget::Granny2FileViewWidget(ESOBrowserMainWindow* window, QWidget* parent) : FileViewWidget(window, parent) {

}

Granny2FileViewWidget::~Granny2FileViewWidget() = default;

QWidget* Granny2FileViewWidget::createViewWidget() {
	auto widget = new ModelViewerWidget(m_window->storage(), this);
	widget->loadSingleModel(m_id);
	return widget;
}

