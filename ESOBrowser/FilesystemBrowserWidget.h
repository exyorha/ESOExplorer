 #ifndef FILESYSTEM_BROWSER_WIDGET_H
#define FILESYSTEM_BROWSER_WIDGET_H

#include <QWidget>

#include "PersistentTabWidget.h"

#include "ui_FilesystemBrowserWidget.h"

class ESOBrowserMainWindow;

class FilesystemBrowserWidget final : public QWidget, public PersistentTabWidget {
	Q_OBJECT
	Q_INTERFACES(PersistentTabWidget)

public:
	explicit FilesystemBrowserWidget(ESOBrowserMainWindow *window, QWidget* parent = nullptr);
	~FilesystemBrowserWidget() override;

	void saveToStream(QDataStream& stream) const override;
	void restoreFromStream(QDataStream& stream) override;

private slots:
	void onOpen();
	void onOpenAsBinary();
	void treeDoubleClicked(const QModelIndex& index);
	void customTreeContextMenuRequested(const QPoint& pos);

private:
	bool getCurrentId(uint64_t &id);
	bool getIdForIndex(const QModelIndex& index, uint64_t &id);

	void openAutodetect(uint64_t id);
	
	template<typename T>
	void open(uint64_t id);

	ESOBrowserMainWindow* m_window;
	Ui::FilesystemBrowserWidget ui;
	QMenu* m_contextMenu;
};

#endif
