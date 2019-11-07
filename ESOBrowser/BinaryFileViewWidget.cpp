#include "BinaryFileViewWidget.h"
#include "HexView.h"
#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"

#include <QMessageBox.h>

BinaryFileViewWidget::BinaryFileViewWidget(ESOBrowserMainWindow* window, QWidget* parent) : FileViewWidget(window, parent) {

}

BinaryFileViewWidget::~BinaryFileViewWidget() = default;

QWidget* BinaryFileViewWidget::createViewWidget() {
	auto widget = new HexView(this);

	auto data = loadData();

	widget->setData(std::make_unique<VectorDataStorage>(std::move(data)));

	return widget;
}

BinaryFileViewWidget::VectorDataStorage::VectorDataStorage(std::vector<unsigned char>&& data) : m_data(data) {

}

BinaryFileViewWidget::VectorDataStorage::~VectorDataStorage() = default;

QByteArray BinaryFileViewWidget::VectorDataStorage::getData(size_t position, size_t length) {
	if (position >= m_data.size())
		return QByteArray();

	length = std::min<size_t>(length, m_data.size() - position);

	return QByteArray::fromRawData(reinterpret_cast<const char*>(m_data.data()) + position, length);
}

size_t BinaryFileViewWidget::VectorDataStorage::size() {
	return m_data.size();
}


