#include "NameHarvestingWidget.h"

#include "ESOBrowserMainWindow.h"
#include "NameHarvestingEngine.h"

NameHarvestingWidget::NameHarvestingWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window), m_engine(window->nameHarvesting()) {
	ui.setupUi(this);

	connect(m_engine, &NameHarvestingEngine::harvestingInProgressChanged, this, &NameHarvestingWidget::onHarvestingInProgressChanged);
	connect(m_engine, &NameHarvestingEngine::harvestingProgress, this, &NameHarvestingWidget::onHarvestingProgressChanged);
	connect(ui.harvest, &QPushButton::toggled, m_engine, &NameHarvestingEngine::setHarvestingInProgress);

	onHarvestingInProgressChanged();
	onHarvestingProgressChanged();
}

NameHarvestingWidget::~NameHarvestingWidget() = default;

void NameHarvestingWidget::saveToStream(QDataStream& stream) const {
	(void)stream;
}

void NameHarvestingWidget::restoreFromStream(QDataStream& stream) {
	(void)stream;
}

void NameHarvestingWidget::onHarvestingInProgressChanged() {
	ui.harvest->setChecked(m_engine->harvestingInProgress());
	if(m_engine->harvestingInProgress())
		ui.rejects->setModel(nullptr);
	else
		ui.rejects->setModel(m_engine->rejectModel());

}

void NameHarvestingWidget::onHarvestingProgressChanged() {
	ui.harvestProgress->setValue(m_engine->filesProcessed());
	ui.harvestProgress->setMaximum(m_engine->filesTotal());
}
