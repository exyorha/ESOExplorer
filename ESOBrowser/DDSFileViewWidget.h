#ifndef DDS_FILE_VIEW_WIDGET_H
#define DDS_FILE_VIEW_WIDGET_H

#include "FileViewWidget.h"

class DDSFileViewWidget final : public FileViewWidget {
	Q_OBJECT

public:
	explicit DDSFileViewWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~DDSFileViewWidget() override;

protected:
	QWidget* createViewWidget() override;

private:
	static void scratchImageReleaseFunction(void* ptr);
};

#endif
