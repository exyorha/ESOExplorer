#ifndef ESO_DATABASE_DEF_MODEL_H
#define ESO_DATABASE_DEF_MODEL_H

#include <QAbstractItemModel>

class ESODatabaseDef;

class ESODatabaseDefModel final : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit ESODatabaseDefModel(const ESODatabaseDef* def, QObject* parent = nullptr);
	~ESODatabaseDefModel() override;

	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent) const override;

private:
	const ESODatabaseDef* m_def;
};

#endif
