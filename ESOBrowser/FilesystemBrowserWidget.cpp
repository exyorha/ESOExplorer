#include "FilesystemBrowserWidget.h"
#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"
#include "FileTypeDetector.h"
#include "BinaryFileViewWidget.h"
#include "TextFileViewWidget.h"
#include "DDSFileViewWidget.h"
#include "Granny2FileViewWidget.h"

#include <QMessageBox>

FilesystemBrowserWidget::FilesystemBrowserWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window) {
	ui.setupUi(this);
	ui.tree->setModel(window->storage()->filesystemModel());
	ui.tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tree, &QTreeView::doubleClicked, this, &FilesystemBrowserWidget::treeDoubleClicked);
	connect(ui.tree, &QTreeView::customContextMenuRequested, this, &FilesystemBrowserWidget::customTreeContextMenuRequested);

	m_contextMenu = new QMenu(this);
	m_contextMenu->setDefaultAction(m_contextMenu->addAction(tr("Open"), this, &FilesystemBrowserWidget::onOpen));
	m_contextMenu->addAction(tr("Open as binary"), this, &FilesystemBrowserWidget::onOpenAsBinary);
}

FilesystemBrowserWidget::~FilesystemBrowserWidget() = default;

void FilesystemBrowserWidget::onOpen() {
	uint64_t id;
	if (getCurrentId(id)) {
		openAutodetect(id);
	}
}

void FilesystemBrowserWidget::onOpenAsBinary() {
	uint64_t id;
	if (getCurrentId(id)) {
		open<BinaryFileViewWidget>(id);
	}
}

void FilesystemBrowserWidget::treeDoubleClicked(const QModelIndex& index) {
	uint64_t id;
	if (getIdForIndex(index, id)) {
		openAutodetect(id);
	}
}

void FilesystemBrowserWidget::customTreeContextMenuRequested(const QPoint& pos) {
	uint64_t id;
	if (getCurrentId(id)) {
		m_contextMenu->popup(ui.tree->viewport()->mapToGlobal(pos));
	}
}

bool FilesystemBrowserWidget::getCurrentId(uint64_t& id) {
	auto current = ui.tree->currentIndex();
	if (!current.isValid())
		return false;

	return getIdForIndex(current, id);
}

bool FilesystemBrowserWidget::getIdForIndex(const QModelIndex &index, uint64_t &id) {
	auto idData = ui.tree->model()->data(index, Qt::UserRole);
	if (idData.isNull())
		return false;

	bool ok;
	id = idData.toULongLong(&ok);
	return ok;
}

void FilesystemBrowserWidget::openAutodetect(uint64_t id) {
	DetectedFileType type = DetectedFileType::Unknown;

	try {
		type = FileTypeDetector().detect(m_window->storage()->filesystem()->readFileByKey(id));
	}
	catch (const std::exception& e) {
		QMessageBox::critical(window(),
			tr("Open Failed"),
			tr("Failed to determine file type: %1").arg(QString::fromStdString(e.what())));
	}

	switch (type) {
	case DetectedFileType::Unknown:
		open<BinaryFileViewWidget>(id);
		break;

	case DetectedFileType::Text:
		open<TextFileViewWidget>(id);
		break;

	case DetectedFileType::DDS:
		open<DDSFileViewWidget>(id);
		break;

	case DetectedFileType::Granny2:
		open<Granny2FileViewWidget>(id);
		break;
	}
}

template<typename T>
void FilesystemBrowserWidget::open(uint64_t id) {
	auto widget = new T(m_window, m_window);
	widget->openFile(id);
	m_window->addTab(widget);
}

void FilesystemBrowserWidget::saveToStream(QDataStream& stream) const {
	(void)stream;
}

void FilesystemBrowserWidget::restoreFromStream(QDataStream& stream) {
	(void)stream;
}
