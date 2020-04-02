#include "DataStorage.h"

#include <QSettings>
#include <QCoreApplication>
#include <QProgressDialog>

#include <fstream>
#include <string>

#include "ESODatabaseDefModel.h"

DataStorage::DataStorage() : m_loadingCancelled(false), m_loadingDialog(nullptr), m_databaseModel(m_depot.database()) {
	std::filesystem::path applicationDirectory(QCoreApplication::applicationDirPath().toStdWString());
	
	m_depot.loadDirectives(applicationDirectory, true);

	m_uiSettings.parseFile(applicationDirectory / "UISettings.dir");
}

DataStorage::~DataStorage() {
	if(m_loadingThread.joinable())
		m_loadingThread.join();
}

bool DataStorage::getDepotPath(std::filesystem::path& path) const {
	QSettings settings;
	auto pathKey = QStringLiteral("data/depotPath");

	if (!settings.contains(pathKey))
		return false;

	auto pathString = settings.value(pathKey).toString();
	path = pathString.toStdWString();

	return true;
}


bool DataStorage::validateDepot(esodata::ValidateDepotResult &result) {
	std::filesystem::path depotPath;

	if (!getDepotPath(depotPath))
		return false;

	m_depot.setDepotPath(std::move(depotPath));

	result = m_depot.validateDepot();

	return true;
}

void DataStorage::setDepotPath(const std::filesystem::path& path) {
	auto pathString = QString::fromStdWString(path.wstring());
	QSettings settings;
	auto pathKey = QStringLiteral("data/depotPath");

	settings.setValue(pathKey, pathString);
}

bool DataStorage::loadDepot() {
	QProgressDialog dialog(QCoreApplication::tr("Loading depot data"), QCoreApplication::tr("Cancel"), 0,
		static_cast<int>(m_depot.getExpectedNumberOfLoadingSteps() + 2));

	connect(&dialog, &QProgressDialog::canceled, this, &DataStorage::loadingCancelled);
	dialog.setAutoReset(false);

	m_loadingDialog = &dialog;

	m_loadingThread = std::thread(&DataStorage::backgroundLoadingThread, this);

	dialog.exec();

	m_loadingThread.join();

	m_loadingDialog = nullptr;

	if (m_loadingException) {
		std::rethrow_exception(m_loadingException);
	}

	return !isLoadingCancelled();
}

bool DataStorage::loadingStepsDone(unsigned int steps) {
	if (isLoadingCancelled())
		return false;

	m_loadingProgress += steps;
	setLoadingProgress(m_loadingProgress);

	return true;
}

void DataStorage::backgroundLoadingThread() {
	try {
		m_loadingProgress = 0;

		m_depot.load(this);

		m_fsModel.buildTree(filesystem());
		if (!loadingStepsDone(1))
			goto finish;
		
		m_fsModel.sortModel();
		if (!loadingStepsDone(1))
			goto finish;

		m_defModels.reserve(database().defs().size());

		for (auto& def : database().defs()) {			
			m_defModels.emplace_back(std::make_unique<ESODatabaseDefModel>(&def, this));
		}
	}

	catch (...) {
		m_loadingException = std::current_exception();
	}

finish:
	QMetaObject::invokeMethod(this, "loadingFinished", Qt::QueuedConnection);
	printf("DONE!\n");
}

void DataStorage::setLoadingProgress(int progress) {
	QMetaObject::invokeMethod(this, "loadingProgress", Qt::QueuedConnection, Q_ARG(int, progress));
}

void DataStorage::loadingFinished() {
	m_loadingDialog->accept();
}

void DataStorage::loadingProgress(int progress) {
	m_loadingDialog->setValue(progress);
}

void DataStorage::loadingCancelled() {
	m_loadingCancelled.store(true);
}

bool DataStorage::isLoadingCancelled() const {
	return m_loadingCancelled.load();
}
