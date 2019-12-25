#ifndef DATABASE_BROWSER_WIDGET_H
#define DATABASE_BROWSER_WIDGET_H

#include <QWidget>
#include "PersistentTabWidget.h"

#include "ui_DatabaseBrowserWidget.h"

class ESOBrowserMainWindow;

class DatabaseBrowserWidget final : public QWidget, public PersistentTabWidget {
	Q_OBJECT
	Q_INTERFACES(PersistentTabWidget)

public:
	explicit DatabaseBrowserWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~DatabaseBrowserWidget() override;

	void saveToStream(QDataStream& stream) const override;
	void restoreFromStream(QDataStream& stream) override;

private slots:
	void on_tables_activated(const QModelIndex& index);
	void on_data_activated(const QModelIndex& index);

private:
	ESOBrowserMainWindow* m_window;
	Ui::DatabaseBrowserWidget ui;
	int m_currentModel;
};

#endif
