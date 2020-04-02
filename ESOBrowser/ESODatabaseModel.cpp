#include "ESODatabaseModel.h"
#include <ESOData/Database/ESODatabase.h>

ESODatabaseModel::ESODatabaseModel(const esodata::ESODatabase* database, QObject* parent) : QAbstractItemModel(parent), m_database(database) {

}

ESODatabaseModel::~ESODatabaseModel() = default;

int ESODatabaseModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;

	return 1;
}

QVariant ESODatabaseModel::data(const QModelIndex& index, int role) const {
	auto def = static_cast<esodata::ESODatabaseDef*>(index.internalPointer());

	switch (role) {
	case Qt::DisplayRole:
		return QString::fromStdString(def->name());

	default:
		return QVariant();
	}
}

QModelIndex ESODatabaseModel::index(int row, int column, const QModelIndex& parent) const {
	if(parent.isValid() || column != 0)
		return QModelIndex();

	return createIndex(row, column, const_cast<esodata::ESODatabaseDef *>(&m_database->defs()[row]));
}

QModelIndex ESODatabaseModel::parent(const QModelIndex& index) const {
	(void)index;
	return QModelIndex();
}

int ESODatabaseModel::rowCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;

	return static_cast<int>(m_database->defs().size());
}

