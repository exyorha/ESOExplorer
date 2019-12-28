#ifndef DEF_SEARCH_WIDGET_H
#define DEF_SEARCH_WIDGET_H

#include <QWidget>
#include "PersistentTabWidget.h"
#include "ui_DefSearchWidget.h"

class ESOBrowserMainWindow;

QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class DefSearchWidget final : public QWidget, public PersistentTabWidget {
	Q_OBJECT
	Q_INTERFACES(PersistentTabWidget)

public:
	explicit DefSearchWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~DefSearchWidget() override;

	void saveToStream(QDataStream& stream) const override;
	void restoreFromStream(QDataStream& stream) override;

private slots:
	void on_search_clicked();

private:
	ESOBrowserMainWindow* m_window;
	Ui::DefSearchWidget ui;
	QStandardItemModel* m_model;
};

#endif
