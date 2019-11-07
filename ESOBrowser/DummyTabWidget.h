#ifndef DUMMY_TAB_WIDGET_H
#define DUMMY_TAB_WIDGET_H

#include <QWidget>

#include "ui_DummyTabWidget.h"

class DummyTabWidget final : public QWidget {
public:
	explicit DummyTabWidget(QWidget *parent = nullptr);
	~DummyTabWidget() override;

private:
	Ui::DummyTabWidget ui;
};

#endif
