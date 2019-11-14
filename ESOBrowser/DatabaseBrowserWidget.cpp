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

}

void DatabaseBrowserWidget::restoreFromStream(QDataStream& stream) {

}

void DatabaseBrowserWidget::on_tables_activated(const QModelIndex& index) {
	ui.data->setModel(m_window->storage()->defModel(index.row()));
}