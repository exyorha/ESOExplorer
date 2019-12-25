#ifndef DATABASE_RECORD_VIEWER_ITEM_DELEGATE_H
#define DATABASE_RECORD_VIEWER_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

class DatabaseRecordViewerItemDelegate final : public QStyledItemDelegate {
	Q_OBJECT

public:
	explicit DatabaseRecordViewerItemDelegate(QObject* parent = nullptr);
	~DatabaseRecordViewerItemDelegate() override;
};

#endif
