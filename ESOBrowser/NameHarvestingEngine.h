#ifndef NAME_HARVESTING_ENGINE_H
#define NAME_HARVESTING_ENGINE_H

#include <QObject>
#include <QThread>
#include <QSqlDatabase>
#include <QStandardItemModel>

#include <unordered_set>

#include "FileNameExtractor.h"
#include "FileNameExtractorCallbacks.h"

class DataStorage;
class NameHarvestingEngineThread;
class NameHarvestingEngineInner;


class NameHarvestingEngine final : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool harvestingInProgress READ harvestingInProgress WRITE setHarvestingInProgress NOTIFY harvestingInProgressChanged)
	Q_PROPERTY(unsigned int filesProcessed READ filesProcessed NOTIFY harvestingProgress)
	Q_PROPERTY(unsigned int filesTotal READ filesTotal NOTIFY harvestingProgress)

public:
	explicit NameHarvestingEngine(DataStorage *storage, QObject* parent = nullptr);
	~NameHarvestingEngine() override;

	inline DataStorage* storage() const {
		return m_storage;
	}

	inline bool harvestingInProgress() const {
		return m_harvestingInProgress;
	}

	inline unsigned int filesProcessed() const {
		return m_filesProcessed;
	}

	inline unsigned int filesTotal() const {
		return m_filesTotal;
	}

	inline QAbstractItemModel* rejectModel() {
		return m_rejectModel;
	}

public slots:
	void setHarvestingInProgress(bool harvestingInProgress);

	void harvestingFinishedInternal();
	void harvestingProgressInternal(unsigned int processed, unsigned int total);
	void processingRejectInternal(const QString& reject);

signals:
	void harvestStartRequestedInternal();
	void harvestStopRequestedInternal();

	void harvestingInProgressChanged();
	void harvestingProgress();

private:
	DataStorage* m_storage;
	NameHarvestingEngineThread* m_engineThread;
	bool m_harvestingInProgress;
	unsigned int m_filesProcessed;
	unsigned int m_filesTotal;
	QStandardItemModel* m_rejectModel;
};

class NameHarvestingEngineThread final : public QThread {
	Q_OBJECT

public:
	explicit NameHarvestingEngineThread(NameHarvestingEngine* owner);
	~NameHarvestingEngineThread() override;

protected:
	void run() override;

private:
	NameHarvestingEngine* m_owner;
	NameHarvestingEngineInner* m_inner;
};

class NameHarvestingEngineInner final : public QObject, private FileNameExtractorCallbacks {
	Q_OBJECT

public:
	explicit NameHarvestingEngineInner(NameHarvestingEngine* owner, QObject* parent = nullptr);
	~NameHarvestingEngineInner() override;

public slots:
	void harvestStart();
	void harvestStop();

signals:
	void harvestFinished();
	void harvestProgress(unsigned int processed, unsigned int total);
	void processingReject(const QString& reject);

private slots:
	void harvestingTick();

private:
	void addFileName(uint64_t id, const std::string& name) override;
	void excludeFile(uint64_t id) override;

	NameHarvestingEngine* m_owner;
	QSqlDatabase m_database;
	bool m_harvesting;
	bool m_transactionOpen;
	bool m_harvestInitialized;
	unsigned int m_filesProcessed;
	unsigned int m_filesTotal;
	std::unordered_set<uint64_t> m_filesToProcessOnThisTick;
	FileNameExtractor m_extractor;
};

#endif
