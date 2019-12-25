#include "DatabaseRecordViewerWidget.h"
#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"

#include <QStandardItemModel>

DatabaseRecordViewerWidget::DatabaseRecordViewerWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window), m_recordId(0) {
	ui.setupUi(this);

	m_model = new QStandardItemModel(this);
	ui.content->setModel(m_model);
}

DatabaseRecordViewerWidget::~DatabaseRecordViewerWidget() = default;

void DatabaseRecordViewerWidget::saveToStream(QDataStream& stream) const {
	stream << QString::fromStdString(m_defName) << m_recordId;
}

void DatabaseRecordViewerWidget::restoreFromStream(QDataStream& stream) {
	QString defName;
	unsigned int recordId;

	stream >> defName >> recordId;

	if (stream.status() == QDataStream::Ok) {
		m_defName = defName.toStdString();
		m_recordId = recordId;
		openRecordInternal();
	}
}

void DatabaseRecordViewerWidget::openRecord(const std::string& defName, unsigned int recordId) {
	m_defName = defName;
	m_recordId = recordId;
	openRecordInternal();
}

void DatabaseRecordViewerWidget::openRecordInternal() {
	qDebug("DatabaseRecordViewerWidget::openRecordInternal(%s, %u)", m_defName.c_str(), m_recordId);

	auto storage = m_window->storage();
	const auto &def = storage->database().findDefByName(m_defName);
	const auto& record = def.findRecordById(m_recordId);

	const auto& name = std::get<std::string>(record.findField("name"));
	setWindowTitle(QString::fromStdString(name));

	for (const auto& fieldName : record.fieldOrder()) {
		auto fieldNameItem = new QStandardItem(QString::fromStdString(fieldName));

		const auto& value = record.findField(fieldName);

		auto items = QList<QStandardItem*>() << fieldNameItem;
		
		auto item = 
			std::visit([this](const auto& value) {
				return convertValueToItem(value);
			}, value);

		if (item)
			items.append(item);

		m_model->appendRow(std::move(items));
	}

	//m_model->appendRow(convert(record));
}


QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(unsigned long long value) {
	return new QStandardItem(QString::fromStdString(std::to_string(value)));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const ESODatabaseRecord::ValueEnum& val) {
	if (std::find(val.definition->values.begin(), val.definition->values.end(), val.value) == val.definition->values.end()) {
		return new QStandardItem(QString::fromStdString(val.definition->name + "::<INVALID ENUM VALUE " + std::to_string(val.value) + ">"));
	}
	else {
		auto it = val.definition->valueNames.find(val.value);
		if (it == val.definition->valueNames.end()) {
			return new QStandardItem(QString::fromStdString(val.definition->name + "::" + std::to_string(val.value)));
		}
		else {
			return new QStandardItem(QString::fromStdString(val.definition->name + "::" + it->second));
		}
	}
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const std::string& value) {
	return new QStandardItem(QString::fromStdString(value));
}
