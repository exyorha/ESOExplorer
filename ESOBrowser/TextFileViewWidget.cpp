#include "TextFileViewWidget.h"

#include <QPlainTextEdit>

TextFileViewWidget::TextFileViewWidget(ESOBrowserMainWindow* window, QWidget* parent) : FileViewWidget(window, parent) {

}

TextFileViewWidget::~TextFileViewWidget() = default;

QWidget* TextFileViewWidget::createViewWidget() {
	auto data = loadData();
	auto editor = new QPlainTextEdit(this);
	editor->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	editor->setReadOnly(true);
	editor->setPlainText(QString::fromLatin1(reinterpret_cast<const char*>(data.data()), data.size()));
	return editor;
}
