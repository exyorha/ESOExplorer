#ifndef FILE_VIEW_WIDGET_H
#define FILE_VIEW_WIDGET_H

#include <qwidget.h>
#include "PersistentTabWidget.h"

#include "ui_FileViewWidget.h"

class ESOBrowserMainWindow;

class FileViewWidget : public QWidget, public PersistentTabWidget {
	Q_OBJECT
	Q_INTERFACES(PersistentTabWidget)

protected:
	explicit FileViewWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);

public:
	~FileViewWidget() override;

	void openFile(uint64_t id);

	void saveToStream(QDataStream& stream) const override;
	void restoreFromStream(QDataStream& stream) override;

protected:
	virtual QWidget *createViewWidget() = 0;

	std::vector<unsigned char> loadData() const;

private slots:
	void on_exportFile_clicked();

protected:
	ESOBrowserMainWindow* m_window;
	Ui::FileViewWidget ui;
	uint64_t m_id;
	QString m_name;
};

#endif
