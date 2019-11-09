#ifndef GRANNY2_FILE_VIEW_WIDGET_H
#define GRANNY2_FILE_VIEW_WIDGET_H

#include "FileViewWidget.h"

class Granny2FileViewWidget final : public FileViewWidget {
	Q_OBJECT

public:
	explicit Granny2FileViewWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~Granny2FileViewWidget() override;

protected:
	QWidget* createViewWidget() override;
};

#endif
