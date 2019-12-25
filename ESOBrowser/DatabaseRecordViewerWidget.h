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

private:
	void openRecordInternal();

	inline QStandardItem* convertValueToItem(const std::monostate&) {
		return nullptr;
	}

	QStandardItem* convertValueToItem(unsigned long long value);

	QStandardItem* convertValueToItem(const ESODatabaseRecord::ValueEnum& value);

	QStandardItem* convertValueToItem(const std::string & value);

	ESOBrowserMainWindow* m_window;
	Ui::DatabaseRecordViewerWidget ui;

	std::string m_defName;
	unsigned int m_recordId;

	QStandardItemModel* m_model;
};

#endif
