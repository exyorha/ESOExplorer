#include "ESODatabaseDefModel.h"
#include "ESODatabaseDef.h"

ESODatabaseDefModel::ESODatabaseDefModel(const ESODatabaseDef* def, QObject* parent) : QAbstractItemModel(parent), m_def(def) {

}

ESODatabaseDefModel::~ESODatabaseDefModel() = default;


int ESODatabaseDefModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;

	return 1;
}

QVariant ESODatabaseDefModel::data(const QModelIndex& index, int role) const {
	auto record = static_cast<ESODatabaseRecord *>(index.internalPointer());

	return QVariant();
}

QModelIndex ESODatabaseDefModel::index(int row, int column, const QModelIndex& parent) const {
	if (parent.isValid() || column != 0)
		return QModelIndex();

	return createIndex(row, column, const_cast<ESODatabaseRecord *>(&m_def->records()[row]));
}

QModelIndex ESODatabaseDefModel::parent(const QModelIndex& index) const {
	(void)index;

	return QModelIndex();
}

int ESODatabaseDefModel::rowCount(const QModelIndex& parent) const {
	if(parent.isValid())
		return 0;

	return static_cast<int>(m_def->records().size());
}

