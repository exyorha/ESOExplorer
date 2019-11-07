#ifndef BINARY_FILE_VIEW_WIDGET_H
#define BINARY_FILE_VIEW_WIDGET_H

#include "FileViewWidget.h"
#include "HexView.h"

class BinaryFileViewWidget final : public FileViewWidget {
	Q_OBJECT

public:
	explicit BinaryFileViewWidget(ESOBrowserMainWindow* window, QWidget* parent = nullptr);
	~BinaryFileViewWidget() override;

protected:
	QWidget *createViewWidget() override;

private:
	class VectorDataStorage final : public HexView::DataStorage {
	public:
		explicit VectorDataStorage(std::vector<unsigned char>&& data);
		~VectorDataStorage() override;

		QByteArray getData(size_t position, size_t length) override;
		size_t size() override;

	private:
		std::vector<unsigned char> m_data;
	};
};

#endif
