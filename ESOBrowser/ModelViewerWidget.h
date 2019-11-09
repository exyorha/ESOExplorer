#ifndef MODEL_VIEWER_WIDGET_H
#define MODEL_VIEWER_WIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <unordered_set>

#include "ui_ModelViewerWidget.h"

class DataStorage;

struct granny_variant;

class ModelViewerWidget final : public QWidget {
	Q_OBJECT

public:
	explicit ModelViewerWidget(DataStorage* storage, QWidget* parent = nullptr);
	~ModelViewerWidget() override;

	void loadSingleModel(uint64_t key);

private:
	QList<QStandardItem*> objectToStructure(granny_variant *variant, const QString& wrapperName);
	QStandardItem* ptrItem(void* ptr);

	Ui::ModelViewerWidget ui;
	DataStorage* m_storage;
	QStandardItemModel* m_structureModel;
	std::unordered_set<void*> m_printedNodes;
};

#endif
