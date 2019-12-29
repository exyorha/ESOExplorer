#ifndef DATABASE_RECORD_VIEWER_WIDGET_H
#define DATABASE_RECORD_VIEWER_WIDGET_H

#include <QWidget>
#include "PersistentTabWidget.h"
#include "ESODatabaseRecord.h"
#include "ui_DatabaseRecordViewerWidget.h"

class ESOBrowserMainWindow;

QT_FORWARD_DECLARE_CLASS(QStandardItem)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class DatabaseRecordViewerWidget final : public QWidget, public PersistentTabWidget {
	Q_OBJECT
	Q_INTERFACES(PersistentTabWidget)

public:
	explicit DatabaseRecordViewerWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~DatabaseRecordViewerWidget() override;

	void saveToStream(QDataStream& stream) const override;
	void restoreFromStream(QDataStream& stream) override;

	void openRecord(const std::string& defName, unsigned int recordId);

private slots:
	void on_content_activated(const QModelIndex& index);

private:
	void openRecordInternal();

	inline QStandardItem* convertValueToItem(const std::monostate&, QStandardItem*) {
		return nullptr;
	}

	QStandardItem* convertValueToItem(long long value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(unsigned long long value, QStandardItem *childReceiver);

	QStandardItem* convertValueToItem(double value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValueEnum& value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValueArray& value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValueForeignKey& value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValueAssetReference& value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(const std::string & value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(bool value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValueStruct & value, QStandardItem* childReceiver);

	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver);
	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver, const std::monostate &);
	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver, uint32_t unknownValue);
	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver, const ESODatabaseRecord::ValueForeignKey &fkey);

	ESOBrowserMainWindow* m_window;
	Ui::DatabaseRecordViewerWidget ui;

	std::string m_defName;
	unsigned int m_recordId;

	QStandardItemModel* m_model;
};

#endif
