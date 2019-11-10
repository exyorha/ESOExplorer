#include "DataStorage.h"

#include <QSettings>
#include <QCoreApplication>
#include <QProgressDialog>

#include <fstream>
#include <string>

DataStorage::DataStorage() : m_loadingCancelled(false), m_loadingDialog(nullptr) {
	std::filesystem::path applicationDirectory(QCoreApplication::applicationDirPath().toStdWString());

	m_supportedVersions.parseFile(applicationDirectory / "SupportedVersions.dir");
	m_filesystem.parseFile(applicationDirectory / "Filesystem.dir");
	m_filenameHarvesting.parseFile(applicationDirectory / "FilenameHarvesting.dir");
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

ValidateDepotResult DataStorage::validateDepot() {
	if (!getDepotPath(m_depotPath))
		return ValidateDepotResult::NotSpecified;

	if (!queryDepotVersion())
		return ValidateDepotResult::DoesNotExist;

	if (std::find(m_supportedVersions.supportedVersions.begin(), m_supportedVersions.supportedVersions.end(), m_depotClientVersion) == m_supportedVersions.supportedVersions.end()) {
		return ValidateDepotResult::UnsupportedVersion;
	}
	else {
		return ValidateDepotResult::Succeeded;
	}
}

void DataStorage::setDepotPath(const std::filesystem::path& path) {
	auto pathString = QString::fromStdWString(path.wstring());
	QSettings settings;
	auto pathKey = QStringLiteral("data/depotPath");

	settings.setValue(pathKey, pathString);
}

bool DataStorage::queryDepotVersion() {
	try {
		std::ifstream stream;
		stream.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
		stream.open(m_depotPath / "depot" / "_databuild" / "databuild.stamp");
		
		std::getline(stream, m_depotBuild);
		std::getline(stream, m_depotBuildDate);

		stream.exceptions(std::ios::failbit | std::ios::badbit);
		std::getline(stream, m_depotClientVersion);

		return true;
	}
	catch (const std::exception&) {
		return false;
	}
}

bool DataStorage::loadDepot() {
	QProgressDialog dialog(QCoreApplication::tr("Loading depot data"), QCoreApplication::tr("Cancel"), 0,
		static_cast<int>(m_filesystem.manifests.size() + m_filesystem.fileTables.size() + 2));

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

void DataStorage::backgroundLoadingThread() {
	try {
		int progress = 0;
		setLoadingProgress(progress);

		for (const auto& manifest : m_filesystem.manifests) {
			if (isLoadingCancelled())
				return;

			m_fs.addManifest(m_depotPath / manifest, false);
			setLoadingProgress(++progress);
		}

		for (auto fileTable : m_filesystem.fileTables) {
			if (isLoadingCancelled())
				return;

			m_fs.loadFileTable(fileTable);
			setLoadingProgress(++progress);
		}

		if (isLoadingCancelled())
			return;

		m_fsModel.buildTree(&m_fs);
		setLoadingProgress(++progress);

		if (isLoadingCancelled())
			return;

		m_fsModel.sortModel();
		setLoadingProgress(++progress);
	}

	catch (...) {
		m_loadingException = std::current_exception();
	}

	QMetaObject::invokeMethod(this, "loadingFinished");
}

void DataStorage::setLoadingProgress(int progress) {
	QMetaObject::invokeMethod(this, "loadingProgress", Q_ARG(int, progress));
}

void DataStorage::loadingFinished() {
	m_loadingDialog->reset();
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
