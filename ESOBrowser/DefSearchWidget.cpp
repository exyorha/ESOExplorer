#include "DefSearchWidget.h"

#include <QStandardItemModel>

#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"

DefSearchWidget::DefSearchWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window) {
	ui.setupUi(this);
	m_model = new QStandardItemModel(this);
	ui.results->setModel(m_model);
	ui.results->setEditTriggers(QAbstractItemView::NoEditTriggers);
	setWindowTitle(tr("Search defs"));

	ui.id->setValidator(new QIntValidator);
}

DefSearchWidget::~DefSearchWidget() = default;

void DefSearchWidget::saveToStream(QDataStream& stream) const {
	(void)stream;
}

void DefSearchWidget::restoreFromStream(QDataStream& stream) {
	(void)stream;
}

void DefSearchWidget::on_search_clicked() {
	bool ok;
	auto id = ui.id->text().toUInt(&ok);

	if (ok) {
		m_model->clear();

		for (const auto& def : m_window->storage()->database().defs()) {
			auto result = def.findRecordById(id);
			if (result) {
				QList<QStandardItem*> items;
				items.append(new QStandardItem(QString::fromStdString(def.name())));
				items.append(new QStandardItem(QString::fromStdString(std::to_string(id))));
				items.append(new QStandardItem(QString::fromStdString(std::get<std::string>(result->findField("name")))));
				m_model->appendRow(items);
			}
		}
	}
}