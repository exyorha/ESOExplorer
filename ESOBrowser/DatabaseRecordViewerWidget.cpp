#include "DatabaseRecordViewerWidget.h"
#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"
#include "DatabaseRecordViewerItemDelegate.h"
#include "FilesystemBrowserWidget.h"

#include <QStandardItemModel>

DatabaseRecordViewerWidget::DatabaseRecordViewerWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window), m_recordId(0) {
	ui.setupUi(this);

	m_model = new QStandardItemModel(this);
	ui.content->setModel(m_model);
	ui.content->setItemDelegate(new DatabaseRecordViewerItemDelegate(ui.content));
	ui.content->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

DatabaseRecordViewerWidget::~DatabaseRecordViewerWidget() = default;

void DatabaseRecordViewerWidget::on_content_activated(const QModelIndex& index) {
	if (index.isValid()) {
		auto def = index.data(Qt::UserRole);
		if (def.isValid()) {
			auto defName = def.toString();

			auto defIndex = index.data(Qt::UserRole + 1).toUInt();

			if (defIndex != 0) {
				auto tab = new DatabaseRecordViewerWidget(m_window, m_window);
				tab->openRecord(defName.toStdString(), defIndex);
				m_window->addTab(tab);
			}
		}
		else {
			auto file = index.data(Qt::UserRole + 2);
			if (file.isValid()) {
				FilesystemBrowserWidget browser(m_window);
				browser.openAutodetect(file.toUInt());
			}
		}
	}
}

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
	auto storage = m_window->storage();
	const auto &def = storage->database().findDefByName(m_defName);
	auto record = def.findRecordById(m_recordId);

	if (!record)
		return;

	const auto& name = std::get<std::string>(record->findField("name"));
	setWindowTitle(QString::fromStdString(name));

	for (const auto& fieldName : record->fieldOrder()) {
		auto fieldNameItem = new QStandardItem(QString::fromStdString(fieldName));

		const auto& value = record->findField(fieldName);

		QList<QStandardItem*> items;
		items << fieldNameItem;
		
		auto item = 
			std::visit([this, fieldNameItem](const auto& value) {
				return convertValueToItem(value, fieldNameItem);
			}, value);

		if (item)
			items.append(item);

		m_model->appendRow(std::move(items));
	}

	//m_model->appendRow(convert(record));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(long long value, QStandardItem* childReceiver) {
	return new QStandardItem(QString::fromStdString(std::to_string(value)));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(unsigned long long value, QStandardItem* childReceiver) {
	return new QStandardItem(QString::fromStdString(std::to_string(value)));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(double value, QStandardItem* childReceiver) {
	return new QStandardItem(QString::fromStdString(std::to_string(value)));
}


QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValueEnum& val, QStandardItem* childReceiver) {
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

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValueArray& value, QStandardItem* childReceiver) {
	for (const auto& subvalue : value.values) {
		auto subitem = new QStandardItem();

		auto result = std::visit([this, subitem](const auto& value) {
			return convertValueToItem(value, subitem);
		}, subvalue);

		QList<QStandardItem *> row;
		row << subitem;

		if (result)
			row << result;

		childReceiver->appendRow(row);
	}

	return nullptr;
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValueForeignKey& value, QStandardItem* childReceiver) {
	if (value.id == 0) {
		return new QStandardItem(QString::fromStdString("Null " + value.def));
	}
	else {
		const auto &def = m_window->storage()->database().findDefByName(value.def);

		auto target = def.findRecordById(value.id);
		if (target == nullptr) {
			return new QStandardItem(QString::fromStdString("BROKEN " + value.def + " " + std::to_string(value.id)));
		}
		else {
			const auto& targetName = std::get<std::string>(target->findField("name"));
			auto item = new QStandardItem;
			item->setData(QString::fromStdString(targetName), Qt::DisplayRole);
			item->setData(QString::fromStdString(def.name()), Qt::UserRole);
			item->setData(QColor(Qt::blue), Qt::ForegroundRole);
			item->setData(value.id, Qt::UserRole + 1);
			return item;
		}
	}
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const std::string& value, QStandardItem* childReceiver) {
	return new QStandardItem(QString::fromStdString(value));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(bool value, QStandardItem *childReceiver) {
	return new QStandardItem(value ? QStringLiteral("true") : QStringLiteral("false"));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValueStruct& record, QStandardItem* childReceiver) {
	for (const auto& fieldName : record.fieldOrder()) {
		auto fieldNameItem = new QStandardItem(QString::fromStdString(fieldName));

		const auto& value = record.findField(fieldName);

		QList<QStandardItem*> items;
		items << fieldNameItem;

		auto item =
			std::visit([this, fieldNameItem](const auto& value) {
			return convertValueToItem(value, fieldNameItem);
		}, value);

		if (item)
			items.append(item);

		childReceiver->appendRow(std::move(items));
	}

	return nullptr;
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValueAssetReference& value, QStandardItem* childReceiver) {
	if (value.id == 0) {
		return new QStandardItem(QStringLiteral("NULL Asset"));
	}
	else {
		auto name = m_window->storage()->filesystemModel()->nameForId(value.id);

		auto item = new QStandardItem;
		item->setData(name, Qt::DisplayRole);
		item->setData(QColor(Qt::darkGreen), Qt::ForegroundRole);
		item->setData(value.id, Qt::UserRole + 2);
		return item;
	}
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver) {
	return std::visit(
		[this, &val, childReceiver](const auto& value) {
			return convertValueToItem(val, childReceiver, value);
		}, val.data);
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver, const std::monostate&) {
	auto it = val.selector.definition->valueNames.find(val.selector.value);
	return new QStandardItem(QString::fromStdString(it->second));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver, uint32_t unknownValue) {
	return new QStandardItem(QStringLiteral("Unresolved polymorphic reference: type %1::%2, ID %3")
		.arg(QString::fromStdString(val.selector.definition->name)).arg(val.selector.value).arg(unknownValue));
}

QStandardItem* DatabaseRecordViewerWidget::convertValueToItem(const esodata::ESODatabaseRecord::ValuePolymorphicReference& val, QStandardItem* childReceiver, const esodata::ESODatabaseRecord::ValueForeignKey& fkey) {
	auto item = convertValueToItem(fkey, childReceiver);

	auto it = val.selector.definition->valueNames.find(val.selector.value);
	auto displayText = item->data(Qt::DisplayRole).toString();
	displayText.prepend(tr("%1: ").arg(QString::fromStdString(it->second)));
	item->setData(displayText, Qt::DisplayRole);

	return item;
}
