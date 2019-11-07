#include "DDSFileViewWidget.h"

#include <DirectXTex.h>

DDSFileViewWidget::DDSFileViewWidget(ESOBrowserMainWindow* window, QWidget* parent) : FileViewWidget(window, parent) {

}

DDSFileViewWidget::~DDSFileViewWidget() = default;


QWidget* DDSFileViewWidget::createViewWidget() {
	auto data = loadData();

	auto image = std::make_unique<DirectX::ScratchImage>();
	auto hr = DirectX::LoadFromDDSMemory(data.data(), data.size(), DirectX::DDS_FLAGS_NONE, nullptr, *image);
	if (FAILED(hr))
		return nullptr;

	if (DirectX::IsCompressed(image->GetMetadata().format)) {
		DirectX::ScratchImage outImage;
		hr = DirectX::Decompress(
			image->GetImages(),
			image->GetImageCount(),
			image->GetMetadata(),
			DXGI_FORMAT_R8G8B8A8_UNORM,
			outImage);

		if (FAILED(hr))
			return nullptr;

		*image = std::move(outImage);
	}

	if (image->GetMetadata().format != DXGI_FORMAT_R8G8B8A8_UNORM) {
		DirectX::ScratchImage outImage;
		hr = DirectX::Convert(
			image->GetImages(),
			image->GetImageCount(),
			image->GetMetadata(),
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DirectX::TEX_FILTER_FANT,
			0.0,
			outImage);

		if (FAILED(hr))
			return nullptr;

		*image = std::move(outImage);
	}

	auto firstImage = image->GetImage(0, 0, 0);
	auto ptr = image.release();
	QImage qImage(
		ptr->GetPixels(),
		firstImage->width,
		firstImage->height,
		firstImage->rowPitch,
		QImage::Format_RGBA8888,
		scratchImageReleaseFunction,
		ptr);

	auto pixmap = QPixmap::fromImage(qImage);

	auto widget = new QLabel(this);
	widget->setPixmap(pixmap);
	widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	return widget;
}

void DDSFileViewWidget::scratchImageReleaseFunction(void* ptr) {
	delete static_cast<DirectX::ScratchImage*>(ptr);
}
