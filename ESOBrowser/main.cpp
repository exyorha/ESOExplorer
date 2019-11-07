#include <QApplication>

#include "ESOBrowserMainWindow.h"
#include "ESOBrowserSelectDepotDialog.h"

#include "DataStorage.h"

#include <QMessageBox>

int main(int argc, char* argv[]) {
	QApplication::setOrganizationName(QStringLiteral("UnwindProject"));
	QApplication::setOrganizationDomain(QStringLiteral("unwind-project.org"));
	QApplication::setApplicationName(QStringLiteral("ESOBrowser"));

	QApplication app(argc, argv);

	MainWindowCommand command = MainWindowCommand::None;
	int result;

	do {

		std::unique_ptr<DataStorage> storage;
		try {
			storage = std::make_unique<DataStorage>();
		}
		catch (const std::exception& e) {
			auto result = QMessageBox::critical(
				nullptr,
				app.translate("main", "ESOBrowser"),
				app.translate("main", "An error has occured during directive parsing. This usually means that one of directive files is either missing or incorrect.\n\n%1")
				.arg(QString::fromLocal8Bit(e.what())), QMessageBox::Abort, QMessageBox::Retry);
			
			if (result == QMessageBox::Abort) {
				return 1;
			}
			else {
				if(command == MainWindowCommand::None)
					command = MainWindowCommand::ReloadDirectives;
				continue;
			}
		}

		auto validationResult = storage->validateDepot();

		while (validationResult != ValidateDepotResult::Succeeded || command == MainWindowCommand::ChangeDepot) {
			if (validationResult == ValidateDepotResult::UnsupportedVersion && command != MainWindowCommand::ChangeDepot) {
				QStringList supportedVersions;
				for (const auto& version : storage->supportedVersions()) {
					supportedVersions.append(QString::fromStdString(version));
				}

				auto result =
					QMessageBox::question(nullptr,
						app.translate("main", "ESOBrowser"),
						app.translate("main", "Depot that you're trying to open is built for an unsupported version of the client. "
							"There may be differences in the file structure major enough to cause errors.\nDo you want to continue anyway?\n\n"
							"Required client version: %1\n"
							"Supported client versions: %2\n")
						.arg(QString::fromStdString(storage->depotClientVersion()))
						.arg(supportedVersions.join(QStringLiteral(", "))),
						QMessageBox::Yes,
						QMessageBox::No);

				if (result == QMessageBox::Yes) {
					validationResult = ValidateDepotResult::Succeeded;
				}
			}

			if (validationResult != ValidateDepotResult::Succeeded || command == MainWindowCommand::ChangeDepot) {
				ESOBrowserSelectDepotDialog selectDialog;
				if (selectDialog.exec() == QDialog::Rejected) {
					if (validationResult == ValidateDepotResult::Succeeded && command == MainWindowCommand::ChangeDepot) {
						break;
					}
					else {
						return 1;
					}
				}

				storage->setDepotPath(selectDialog.path());
				validationResult = storage->validateDepot();
			}

			if (validationResult == ValidateDepotResult::Succeeded) {
				command = MainWindowCommand::None;
			}
		}

		try {
			if (!storage->loadDepot())
				return 1;
		}
		catch (const std::exception& e) {
			auto result = QMessageBox::critical(
				nullptr,
				app.translate("main", "ESOBrowser"),
				app.translate("main", "An error has occured during storage initialization.\n\n%1")
				.arg(QString::fromLocal8Bit(e.what())), QMessageBox::Abort, QMessageBox::Retry);

			if (result == QMessageBox::Abort) {
				return 1;
			}
			else {
				if (command == MainWindowCommand::None)
					command = MainWindowCommand::ReloadDirectives;
				continue;
			}
		}

		command = MainWindowCommand::None;

		ESOBrowserMainWindow win(storage.get());
		win.show();

		result = app.exec();

		command = win.command();

	} while (command != MainWindowCommand::None);

	return result;
}