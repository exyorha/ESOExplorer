#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <filesystem>
#include <thread>
#include <atomic>

#include <QObject>

#include "SupportedVersionsDirectiveFile.h"
#include "FilesystemDirectiveFile.h"
#include "ESOFilesystemModel.h"
#include "FilenameHarvestingDirectiveFile.h"
#include "ESODatabase.h"
#include "ESODatabaseModel.h"
#include "UISettingsDirectiveFile.h"

#include <ESOData/Filesystem/Filesystem.h>

QT_FORWARD_DECLARE_CLASS(QProgressDialog);

class ESODatabaseDefModel;

enum class ValidateDepotResult {
	Succeeded,
	NotSpecified,
	DoesNotExist,
	UnsupportedVersion
};

class DataStorage final : public QObject {
	Q_OBJECT

public:
	DataStorage();
	~DataStorage();

	DataStorage(DataStorage& other) = delete;
	DataStorage &operator =(DataStorage& other) = delete;

	ValidateDepotResult validateDepot();

	inline const std::vector<std::string> & supportedVersions() const {
		return m_supportedVersions.supportedVersions;
	}

	inline const std::vector<std::string>& prefixesForFilenameHarvesting() const {
		return m_filenameHarvesting.prefixes;
	}

	inline const std::vector<std::string> defFieldsAsColumns() const {
		return m_uiSettings.defFieldsAsColumns;
	}

	inline const std::string& depotBuild() const {
		return m_depotBuild;
	}

	inline const std::string& depotBuildDate() const {
		return m_depotBuildDate;
	}

	inline const std::string& depotClientVersion() const {
		return m_depotClientVersion;
	}

	void setDepotPath(const std::filesystem::path& path);

	bool loadDepot();

	inline const esodata::Filesystem* filesystem() const {
		return &m_fs;
	}

	inline ESOFilesystemModel* filesystemModel() {
		return &m_fsModel;
	}

	inline const ESODatabase& database() const {
		return m_database;
	}

	inline ESODatabaseModel* databaseModel() {
		return &m_databaseModel;
	}

	inline ESODatabaseDefModel* defModel(size_t index) {
		return m_defModels[index].get();
	}

private slots:
	void loadingFinished();
	void loadingCancelled();
	void loadingProgress(int progress);

private:
	bool getDepotPath(std::filesystem::path& path) const;
	bool queryDepotVersion();

	void backgroundLoadingThread();

	bool isLoadingCancelled() const;
	void setLoadingProgress(int progress);

	SupportedVersionsDirectiveFile m_supportedVersions;
	FilesystemDirectiveFile m_filesystem;
	FilenameHarvestingDirectiveFile m_filenameHarvesting;
	UISettingsDirectiveFile m_uiSettings;

	std::filesystem::path m_depotPath;
	std::string m_depotBuild;
	std::string m_depotBuildDate;
	std::string m_depotClientVersion;

	std::thread m_loadingThread;
	std::exception_ptr m_loadingException;
	std::atomic<bool> m_loadingCancelled;
	QProgressDialog* m_loadingDialog;
	
	esodata::Filesystem m_fs;
	ESOFilesystemModel m_fsModel;

	ESODatabase m_database;
	ESODatabaseModel m_databaseModel;
	std::vector<std::unique_ptr<ESODatabaseDefModel>> m_defModels;
};

#endif
