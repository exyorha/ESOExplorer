#include "ESOBrowserSelectDepotDialog.h"

#include <QFileDialog>
#include <QSettings>

ESOBrowserSelectDepotDialog::ESOBrowserSelectDepotDialog(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);

	for (const auto& path : { QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Zenimax_Online\\Launcher"), QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\WOW6432Node\\Zenimax_Online\\Launcher") }) {
		QSettings settings(path, QSettings::NativeFormat);
		auto value = settings.value(QStringLiteral("InstallPath"));
		if (!value.isNull()) {
			auto installPath = value.toString() + QStringLiteral("\\The Elder Scrolls Online");
			ui.path->setText(installPath);

			break;
		}
	}
}

ESOBrowserSelectDepotDialog::~ESOBrowserSelectDepotDialog() = default;

void ESOBrowserSelectDepotDialog::on_pathBrowse_clicked() {
	auto dir = QFileDialog::getExistingDirectory(this, tr("Browse for Depot"), ui.path->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty()) {
		ui.path->setText(dir);
	}
}

std::filesystem::path ESOBrowserSelectDepotDialog::path() const {
	return ui.path->text().toStdWString();
}
