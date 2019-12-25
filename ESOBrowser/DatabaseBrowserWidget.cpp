#include "DatabaseBrowserWidget.h"
#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"
#include "ESODatabaseDefModel.h"
#include "DatabaseRecordViewerWidget.h"

DatabaseBrowserWidget::DatabaseBrowserWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window), m_currentModel(-1) {
	ui.setupUi(this);
	ui.tables->setModel(window->storage()->databaseModel());
}

DatabaseBrowserWidget::~DatabaseBrowserWidget() = default;

void DatabaseBrowserWidget::saveToStream(QDataStream& stream) const {
	stream << ui.splitter->saveState();
	stream << m_currentModel;
}

void DatabaseBrowserWidget::restoreFromStream(QDataStream& stream) {
	QByteArray splitter;
	int currentModel;
	stream >> splitter;
	stream >> currentModel;

	if (stream.status() == QDataStream::Ok) {
		ui.splitter->restoreState(splitter);

		m_currentModel = currentModel;

		if (m_currentModel < 0) {
			ui.data->setModel(nullptr);
		}
		else {
			ui.data->setModel(m_window->storage()->defModel(currentModel));
		}
	}

}

void DatabaseBrowserWidget::on_tables_activated(const QModelIndex& index) {
	if (index.isValid()) {
		m_currentModel = index.row();
		ui.data->setModel(m_window->storage()->defModel(m_currentModel));
	}
	else {
		m_currentModel = -1;
		ui.data->setModel(nullptr);
	}
}

void DatabaseBrowserWidget::on_data_activated(const QModelIndex& index) {
	if (index.isValid()) {
		const auto &def = m_window->storage()->defModel(m_currentModel)->def()->name();
		auto id = ui.data->model()->data(index.siblingAtColumn(0), Qt::DisplayRole).toString().toUInt();

		auto widget = new DatabaseRecordViewerWidget(m_window, m_window);
		widget->openRecord(def, id);
		m_window->addTab(widget);
	}
}
