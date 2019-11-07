#ifndef TEXT_FILE_VIEW_WIDGET_H
#define TEXT_FILE_VIEW_WIDGET_H

#include "FileViewWidget.h"

class TextFileViewWidget final : public FileViewWidget {
	Q_OBJECT

public:
	explicit TextFileViewWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~TextFileViewWidget() override;

protected:
	QWidget* createViewWidget() override;
};

#endif
