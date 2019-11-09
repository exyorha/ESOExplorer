#ifndef MODEL_VIEWER_WIDGET_H
#define MODEL_VIEWER_WIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <unordered_set>

#include "ui_ModelViewerWidget.h"

class DataStorage;
class ESOBrowserMainWindow;
class Granny2Renderable;
class FilamentViewport;

struct granny_variant;
struct granny_data_type_definition;

class ModelViewerWidget final : public QWidget {
	Q_OBJECT

public:
	explicit ModelViewerWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~ModelViewerWidget() override;

	void loadSingleModel(uint64_t key);

private:
	QList<QStandardItem*> objectToStructure(granny_variant *variant, const QString& wrapperName);
	QStandardItem* ptrItem(void* ptr);

	QList<QStandardItem*> singleObjectToStructure(granny_data_type_definition* type, void*& data);

	Ui::ModelViewerWidget ui;
	ESOBrowserMainWindow* m_window;
	QStandardItemModel* m_structureModel;
	std::unordered_set<void*> m_printedNodes;
	std::shared_ptr<Granny2Renderable> m_model;
	FilamentViewport* m_viewport;
};

#endif
