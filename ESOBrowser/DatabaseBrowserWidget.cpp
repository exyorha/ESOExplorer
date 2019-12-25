#include "DatabaseBrowserWidget.h"
#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"
#include "ESODatabaseDefModel.h"

DatabaseBrowserWidget::DatabaseBrowserWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window) {
	ui.setupUi(this);
	ui.tables->setModel(window->storage()->databaseModel());
}

DatabaseBrowserWidget::~DatabaseBrowserWidget() = default;

void DatabaseBrowserWidget::saveToStream(QDataStream& stream) const {
	stream << ui.splitter->saveState();
}

void DatabaseBrowserWidget::restoreFromStream(QDataStream& stream) {
	QByteArray splitter;
	stream >> splitter;
	ui.splitter->restoreState(splitter);
}

void DatabaseBrowserWidget::on_tables_activated(const QModelIndex& index) {
	ui.data->setModel(m_window->storage()->defModel(index.row()));
}