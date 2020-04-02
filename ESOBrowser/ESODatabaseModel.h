#ifndef ESO_DATABASE_MODEL_H
#define ESO_DATABASE_MODEL_H

#include <QAbstractItemModel>

namespace esodata {
	class ESODatabase;
}

class ESODatabaseModel final : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit ESODatabaseModel(const esodata::ESODatabase* database, QObject* parent = nullptr);
	~ESODatabaseModel() override;

	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent) const override;

private:
	const esodata::ESODatabase* m_database;
};

#endif
