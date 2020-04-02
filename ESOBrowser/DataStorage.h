#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <filesystem>
#include <thread>
#include <atomic>

#include <QObject>

#include <ESOData/Depot/ESODepot.h>
#include <ESOData/Depot/IDepotLoadingCallback.h>

#include "ESOFilesystemModel.h"
#include "ESODatabaseModel.h"
#include "UISettingsDirectiveFile.h"

QT_FORWARD_DECLARE_CLASS(QProgressDialog);

class ESODatabaseDefModel;

class DataStorage final : public QObject, protected esodata::IDepotLoadingCallback {
	Q_OBJECT

public:
	DataStorage();
	~DataStorage();

	DataStorage(DataStorage& other) = delete;
	DataStorage &operator =(DataStorage& other) = delete;

	bool validateDepot(esodata::ValidateDepotResult &result);

	inline const std::vector<std::string> & supportedVersions() const {
		return m_depot.supportedVersions();
	}

	inline const std::vector<std::string>& prefixesForFilenameHarvesting() const {
		return m_depot.prefixesForFilenameHarvesting();
	}

	inline const std::vector<std::string> defFieldsAsColumns() const {
		return m_uiSettings.defFieldsAsColumns;
	}

	inline const std::string& depotBuild() const {
		return m_depot.depotBuild();
	}

	inline const std::string& depotBuildDate() const {
		return m_depot.depotBuildDate();
	}

	inline const std::string& depotClientVersion() const {
		return m_depot.depotClientVersion();
	}

	void setDepotPath(const std::filesystem::path& path);

	bool loadDepot();

	inline const esodata::Filesystem* filesystem() const {
		return m_depot.filesystem();
	}

	inline ESOFilesystemModel* filesystemModel() {
		return &m_fsModel;
	}

	inline const esodata::ESODatabase& database() const {
		return *m_depot.database();
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

protected:
	bool loadingStepsDone(unsigned int steps) override;

private:
	bool getDepotPath(std::filesystem::path& path) const;

	void backgroundLoadingThread();

	bool isLoadingCancelled() const;
	void setLoadingProgress(int progress);

	esodata::ESODepot m_depot;

	UISettingsDirectiveFile m_uiSettings;
	
	std::thread m_loadingThread;
	std::exception_ptr m_loadingException;
	std::atomic<bool> m_loadingCancelled;
	QProgressDialog* m_loadingDialog;
	
	ESOFilesystemModel m_fsModel;

	ESODatabaseModel m_databaseModel;
	std::vector<std::unique_ptr<ESODatabaseDefModel>> m_defModels;

	unsigned int m_loadingProgress;
};

#endif
