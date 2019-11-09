#include "ModelViewerWidget.h"
#include "DataStorage.h"
#include "ESOBrowserMainWindow.h"
#include "Granny2TypeHelpers.h"
#include "Granny2Model.h"
#include "Granny2Renderable.h"
#include "FilamentViewport.h"

#include <filament/Scene.h>

#include <granny.h>

ModelViewerWidget::ModelViewerWidget(ESOBrowserMainWindow *window, QWidget* parent) : QWidget(parent), m_window(window) {
	ui.setupUi(this);

	m_structureModel = new QStandardItemModel(this);
	ui.structure->setModel(m_structureModel);

	m_viewport = new FilamentViewport();
	auto container = QWidget::createWindowContainer(m_viewport, ui.viewport);

	auto layout = new QVBoxLayout(ui.viewport);
	layout->addWidget(container);
	ui.viewport->setLayout(layout);

	m_viewport->initialize(window->filament());
}

ModelViewerWidget::~ModelViewerWidget() = default;

void ModelViewerWidget::loadSingleModel(uint64_t key) {
	try {
		auto data = m_window->storage()->filesystem()->readFileByKey(key);

		auto rawFile = GrannyReadEntireFileFromMemory(data.size(), data.data());
		if (!rawFile)
			throw std::logic_error("GrannyReadEntireFileFromMemory failed");

		GrannyFile file(rawFile);

		granny_variant root;
		GrannyGetDataTreeFromFile(file.get(), &root);
		if (root.Type && root.Object) {
			m_structureModel->appendRow(objectToStructure(&root, m_window->storage()->filesystemModel()->nameForId(key)));
		}

		m_model = m_window->filament()->loadModel(key)->createInstance();
		m_viewport->scene()->addEntity(m_model->entity);
	}
	catch (const std::exception& e) {
		ui.notification->setText(tr("Failed to load model: %1").arg(QString::fromStdString(e.what())));
	}
}

QStandardItem* ModelViewerWidget::ptrItem(void* ptr) {
	return new QStandardItem(QString().setNum(reinterpret_cast<uintptr_t>(ptr), 16).rightJustified(16, '0'));
}

QList<QStandardItem*> ModelViewerWidget::singleObjectToStructure(granny_data_type_definition *type, void *&object) {

	auto itemNode = new QStandardItem(QString::fromLatin1(type->Name));
	QStandardItem* itemType = new QStandardItem(QStringLiteral("%1").arg(static_cast<int>(type->Type)));
	QStandardItem* itemVal = nullptr;

	switch (type->Type) {
	case GrannyInlineMember:
	{
		granny_variant var;
		var.Object = object;
		var.Type = type->ReferenceType;
		itemNode->appendRow(objectToStructure(&var, QString()));
		break;
	}

	case GrannyReferenceMember:
	{
		granny_variant var;
		var.Object = *static_cast<void**>(object);
		var.Type = type->ReferenceType;

		if (!var.Object) {
			itemVal = new QStandardItem(QStringLiteral("nullptr"));
		}
		else {
			auto result = m_printedNodes.emplace(var.Object);
			if (result.second) {
				itemNode->appendRow(objectToStructure(&var, QString()));
			}
			else {
				itemVal = ptrItem(var.Object);
			}
		}
		break;
	}

	case GrannyReferenceToArrayMember:
	{
		auto count = *static_cast<granny_int32*>(object);
		auto data = *reinterpret_cast<void**>(static_cast<uint8_t*>(object) + sizeof(granny_int32));

		granny_variant var;
		var.Type = type->ReferenceType;
		var.Object = data;

		for (int index = 0; index < count; index++) {
			itemNode->appendRow(objectToStructure(&var, QStringLiteral("[%1]").arg(index)));
		}
		break;
	}

	case GrannyArrayOfReferencesMember:
	{
		auto count = *static_cast<granny_int32*>(object);
		auto data = *reinterpret_cast<void***>(static_cast<uint8_t*>(object) + sizeof(granny_int32));

		granny_variant var;
		var.Type = type->ReferenceType;

		for (int index = 0; index < count; index++) {
			if (!data[index]) {
				auto header = new QStandardItem(QStringLiteral("[%1]").arg(index));
				auto value = new QStandardItem(QStringLiteral("nullptr"));
				itemNode->appendRow(QList<QStandardItem*>() << header << value);
			}
			else {
				var.Object = data[index];
				auto result = m_printedNodes.emplace(data[index]);
				if (result.second) {
					itemNode->appendRow(objectToStructure(&var, QStringLiteral("[%1]").arg(index)));
				}
				else {
					auto header = new QStandardItem(QStringLiteral("[%1]").arg(index));
					auto value = ptrItem(data[index]);
					itemNode->appendRow(QList<QStandardItem*>() << header << value);
				}
			}
		}
		break;
	}

	case GrannyVariantReferenceMember:
	{
		auto pvar = static_cast<granny_variant*>(object);
		auto var = *pvar;

		if (var.Object && var.Type) {
			itemNode->appendRow(objectToStructure(&var, QStringLiteral("")));
		}
		else {
			itemVal = new QStandardItem(QStringLiteral("nullptr"));
		}
		break;
	}

	case GrannyReferenceToVariantArrayMember:
	{
		auto vartype = *static_cast<granny_data_type_definition * *>(object);
		auto count = *reinterpret_cast<granny_int32*>(static_cast<uint8_t*>(object) + sizeof(granny_data_type_definition*));
		auto data = *reinterpret_cast<void**>(static_cast<uint8_t*>(object) + sizeof(granny_data_type_definition*) + sizeof(granny_int32));

		if (vartype) {
			granny_variant var;
			var.Type = vartype;
			var.Object = data;

			for (int index = 0; index < count; index++) {
				itemNode->appendRow(objectToStructure(&var, QStringLiteral("[%1]").arg(index)));
			}
		}

		break;
	}

	case GrannyStringMember:
	{
		auto string = *static_cast<char**>(object);
		itemVal = new QStandardItem(string ? QString::fromUtf8(string) : QStringLiteral("nullptr"));
		break;
	}

	case GrannyTransformMember:
	{
		auto transform = static_cast<granny_transform*>(object);
		itemVal = new QStandardItem(
			QStringLiteral("T%1 (%2 %3 %4) (%5 %6 %7 %8) [(%9 %10 %11) (%12 %13 %14) (%15 %16 %17)]")
			.arg(transform->Flags)
			.arg(transform->Position[0])
			.arg(transform->Position[1])
			.arg(transform->Position[2])
			.arg(transform->Orientation[0])
			.arg(transform->Orientation[1])
			.arg(transform->Orientation[2])
			.arg(transform->Orientation[3])
			.arg(transform->ScaleShear[0][0])
			.arg(transform->ScaleShear[0][1])
			.arg(transform->ScaleShear[0][2])
			.arg(transform->ScaleShear[1][0])
			.arg(transform->ScaleShear[1][1])
			.arg(transform->ScaleShear[1][2])
			.arg(transform->ScaleShear[2][0])
			.arg(transform->ScaleShear[2][1])
			.arg(transform->ScaleShear[2][2]));
		break;
	}

	case GrannyReal32Member:
		itemVal = new QStandardItem(QStringLiteral("%1").arg(*static_cast<float*>(object)));
		break;

	case GrannyInt8Member:
	case GrannyBinormalInt8Member:
		itemVal = new QStandardItem(QStringLiteral("%1").arg(*static_cast<int8_t*>(object)));
		break;

	case GrannyUInt8Member:
	case GrannyNormalUInt8Member:
		itemVal = new QStandardItem(QStringLiteral("%1").arg(*static_cast<uint8_t*>(object)));
		break;

	case GrannyInt16Member:
	case GrannyBinormalInt16Member:
		itemVal = new QStandardItem(QStringLiteral("%1").arg(*static_cast<int16_t*>(object)));
		break;

	case GrannyUInt16Member:
	case GrannyNormalUInt16Member:
		itemVal = new QStandardItem(QStringLiteral("%1").arg(*static_cast<uint16_t*>(object)));
		break;

	case GrannyInt32Member:
		itemVal = new QStandardItem(QStringLiteral("%1").arg(*static_cast<int32_t*>(object)));
		break;

	case GrannyUInt32Member:
		itemVal = new QStandardItem(QStringLiteral("%1").arg(*static_cast<uint32_t*>(object)));
		break;

	case GrannyReal16Member:
	{
		float val;
		GrannyReal16ToReal32(*static_cast<granny_real16*>(object), &val);
		itemVal = new QStandardItem(QStringLiteral("%1").arg(val));
		break;
	}

	default:
		itemType = new QStandardItem(QStringLiteral("unknown type %1").arg(static_cast<int>(type->Type)));
		break;
	}

	QList<QStandardItem*> itemRow;
	itemRow << itemNode;
	if (itemType)
		itemRow << itemType;
	if (itemVal)
		itemRow << itemVal;

	return itemRow;
}

QList<QStandardItem*>ModelViewerWidget::objectToStructure(granny_variant* variant, const QString& wrapperName) {
	auto thisItem = new QStandardItem(wrapperName);
	auto itemType = new QStandardItem(QStringLiteral("object"));
	auto ptr = ptrItem(variant->Object);

	auto type = variant->Type;
	auto data = variant->Object;

	while (type->Type != GrannyEndMember) {
		auto length = GrannyGetMemberTypeSize(type);

		auto object = data;
		data = static_cast<uint8_t*>(data) + length;

		auto arrayWidth = GrannyGetMemberArrayWidth(type);
		if (arrayWidth == 1) {
			thisItem->appendRow(singleObjectToStructure(type, object));
		}
		else {
			auto arrayItem = new QStandardItem(QString());
			for (int index = 0; index < arrayWidth; index++) {
				arrayItem->appendRow(singleObjectToStructure(type, object));
			}
			thisItem->appendRow(arrayItem);
		}


		type++;
	}

	variant->Object = data;
	
	return QList<QStandardItem*>() << thisItem << itemType << ptr;
}

