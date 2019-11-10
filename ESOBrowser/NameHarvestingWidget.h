#ifndef NAME_HARVESTING_WIDGET_H
#define NAME_HARVESTING_WIDGET_H

#include <QWidget>

#include "PersistentTabWidget.h"
#include "ui_NameHarvestingWidget.h"

class ESOBrowserMainWindow;
class NameHarvestingEngine;

class NameHarvestingWidget final : public QWidget, public PersistentTabWidget {
	Q_OBJECT
	Q_INTERFACES(PersistentTabWidget)

public:
	explicit NameHarvestingWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~NameHarvestingWidget() override;

	void saveToStream(QDataStream& stream) const override;
	void restoreFromStream(QDataStream& stream) override;

private slots:
	void onHarvestingInProgressChanged();
	void onHarvestingProgressChanged();

private:
	ESOBrowserMainWindow* m_window;
	NameHarvestingEngine* m_engine;
	Ui::NameHarvestingWidget ui;
};

#endif
