#include "FileViewWidget.h"
#include "ESOBrowserMainWindow.h"
#include "DataStorage.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocument>

#include <fstream>

FileViewWidget::FileViewWidget(ESOBrowserMainWindow* window, QWidget* parent) : QWidget(parent), m_window(window), m_id(0) {
	ui.setupUi(this);
}

FileViewWidget::~FileViewWidget() = default;

void FileViewWidget::openFile(uint64_t id) {
	m_id = id;
	m_name = m_window->storage()->filesystemModel()->nameForId(m_id);

	QUrl url;
	url.setScheme("esobrowser");
	url.setHost(metaObject()->className());
	url.setPath(m_name);

	auto text = QStringLiteral("<a href=\"%1\">%2</a>").arg(url.toString().toHtmlEscaped()).arg(m_name.toHtmlEscaped());
	ui.fileName->setText(text);

	setWindowTitle(m_name);

	auto widget = createViewWidget();
	if (widget) {
		auto layout = new QVBoxLayout(ui.viewContainer);
		layout->addWidget(widget);
		ui.viewContainer->setLayout(layout);
	}
}

void FileViewWidget::on_exportFile_clicked() {
	auto lastSlash = m_name.lastIndexOf('/');
	QString baseName;
	if (lastSlash < 0) {
		baseName = m_name;
	}
	else {
		baseName = m_name.mid(lastSlash + 1);
	}

	auto name = QFileDialog::getSaveFileName(window(),
		tr("Export File"),
		baseName);

	if (!name.isEmpty()) {
		try {
			auto data = m_window->storage()->filesystem()->readFileByKey(m_id);
			std::ofstream stream;
			stream.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
			stream.open(name.toStdWString(), std::ios::out | std::ios::trunc | std::ios::binary);
			stream.write(reinterpret_cast<char*>(data.data()), data.size());
		}
		catch (const std::exception& e) {
			QMessageBox::critical(window(),
				tr("Open Failed"),
				tr("Failed to export file: %1").arg(QString::fromStdString(e.what())));
		}
	}
}

std::vector<unsigned char> FileViewWidget::loadData() const {
	std::vector<unsigned char> data;

	try {
		return m_window->storage()->filesystem()->readFileByKey(m_id);
	}
	catch (const std::exception& e) {
		QMessageBox::critical(window(),
			tr("Read Failed"),
			tr("Failed to read %2: %1").arg(QString::fromStdString(e.what())).arg(m_name));

		return {};
	}
}

void FileViewWidget::saveToStream(QDataStream& stream) const {
	stream << m_id;
}

void FileViewWidget::restoreFromStream(QDataStream& stream) {
	uint64_t id;
	stream >> id;

	openFile(id);
}
