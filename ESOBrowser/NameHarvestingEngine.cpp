#include "NameHarvestingEngine.h"
#include "DataStorage.h"

#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

#include <sstream>

NameHarvestingEngine::NameHarvestingEngine(DataStorage* storage, QObject* parent) : QObject(parent), m_storage(storage), m_harvestingInProgress(false), m_filesProcessed(0), m_filesTotal(0) {
	m_rejectModel = new QStandardItemModel(this);

	m_engineThread = new NameHarvestingEngineThread(this);
	m_engineThread->start();
}

NameHarvestingEngine::~NameHarvestingEngine() {
	setHarvestingInProgress(false);
	m_engineThread->requestInterruption();
	m_engineThread->quit();
	m_engineThread->wait();
}

void NameHarvestingEngine::setHarvestingInProgress(bool harvestingInProgress) {
	if (m_harvestingInProgress != harvestingInProgress) {
		m_harvestingInProgress = harvestingInProgress;
		if (harvestingInProgress) {
			emit harvestStartRequestedInternal();
		}
		else {
			emit harvestStopRequestedInternal();
		}

		emit harvestingInProgressChanged();
	}
}

void NameHarvestingEngine::harvestingFinishedInternal() {
	if (m_harvestingInProgress) {
		m_harvestingInProgress = false;
		emit harvestingInProgressChanged();
	}
}

void NameHarvestingEngine::harvestingProgressInternal(unsigned int processed, unsigned int total) {
	if (m_filesProcessed != processed || m_filesTotal != total) {
		m_filesProcessed = processed;
		m_filesTotal = total;
		emit harvestingProgress();
	}
}

void NameHarvestingEngine::processingRejectInternal(const QString& reject) {
	auto item = new QStandardItem(reject);
	m_rejectModel->appendRow(item);
}

NameHarvestingEngineThread::NameHarvestingEngineThread(NameHarvestingEngine* owner) : QThread(owner), m_owner(owner), m_inner(nullptr) {

}

NameHarvestingEngineThread::~NameHarvestingEngineThread() = default;

void NameHarvestingEngineThread::run() {
	m_inner = new NameHarvestingEngineInner(m_owner);

	connect(m_owner, &NameHarvestingEngine::harvestStartRequestedInternal, m_inner, &NameHarvestingEngineInner::harvestStart);
	connect(m_owner, &NameHarvestingEngine::harvestStopRequestedInternal, m_inner, &NameHarvestingEngineInner::harvestStop);
	connect(m_inner, &NameHarvestingEngineInner::harvestFinished, m_owner, &NameHarvestingEngine::harvestingFinishedInternal);
	connect(m_inner, &NameHarvestingEngineInner::harvestProgress, m_owner, &NameHarvestingEngine::harvestingProgressInternal);
	connect(m_inner, &NameHarvestingEngineInner::processingReject, m_owner, &NameHarvestingEngine::processingRejectInternal);

	QThread::run();

	delete m_inner;
}

NameHarvestingEngineInner::NameHarvestingEngineInner(NameHarvestingEngine *owner, QObject* parent) : QObject(parent), m_owner(owner), m_harvesting(false),
	m_transactionOpen(false), m_harvestInitialized(false), m_filesProcessed(0), m_filesTotal(0), m_extractor(owner->storage()->filesystem(), this, owner->storage()->prefixesForFilenameHarvesting()) {

	m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("NameHarvestingEngine"));
	m_database.setDatabaseName(QCoreApplication::applicationDirPath() + QStringLiteral("/nameharvesting.db"));
	if (!m_database.open())
		qFatal("QSqlDatabase::open: %s", qPrintable(m_database.lastError().text()));

	bool ok = m_database.exec(QStringLiteral(R"SQL(
		CREATE TABLE IF NOT EXISTS DepotVersion (
			Id INTEGER NOT NULL PRIMARY KEY,
			Version TEXT NOT NULL
		)
	)SQL")).isActive();
	if(!ok)
		qFatal("CREATE TABLE(DepotVersion): %s", qPrintable(m_database.lastError().text()));


	ok = m_database.exec(QStringLiteral(R"SQL(
		CREATE TABLE IF NOT EXISTS Files (
			Id INTEGER NOT NULL PRIMARY KEY
		)
	)SQL")).isActive();

	if (!ok)
		qFatal("CREATE TABLE(Files): %s", qPrintable(m_database.lastError().text()));

	ok = m_database.exec(QStringLiteral(R"SQL(
		CREATE TABLE IF NOT EXISTS FileNames (
			Id INTEGER NOT NULL PRIMARY KEY,
			Name TEXT NOT NULL,
			Harvested INTEGER NOT NULL
		)
	)SQL")).isActive();

	if (!ok)
		qFatal("CREATE TABLE(FileNames): %s", qPrintable(m_database.lastError().text()));

	ok = m_database.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS FileNamesHarvested ON FileNames (Harvested)")).isActive();
	if (!ok)
		qFatal("CREATE INDEX(FileNamesHarvested): %s", qPrintable(m_database.lastError().text()));

	auto storage = owner->storage();

	auto query = m_database.exec("SELECT Version FROM DepotVersion");
	if(!query.isActive())
		qFatal("SELECT DepotVersion: %s", qPrintable(m_database.lastError().text()));

	auto expectedVersion = QString::fromStdString(storage->depotBuild());
	if (!query.first() || query.value(0).toString() != expectedVersion) {
		m_database.transaction();

		QSqlQuery versionQuery(m_database);
		versionQuery.prepare(QStringLiteral("INSERT OR REPLACE INTO DepotVersion (Id, Version) VALUES (1, ?)"));
		versionQuery.addBindValue(expectedVersion);

		bool ok = versionQuery.exec();

		if(!ok)
			qFatal("INSERT DepotVersion: %s", qPrintable(versionQuery.lastError().text()));

		ok = m_database.exec(QStringLiteral("DELETE FROM Files")).isActive();
		if(!ok)
			qFatal("TRUNCATE TABLE Files: %s", qPrintable(m_database.lastError().text()));

		ok = m_database.exec(QStringLiteral("DELETE FROM FileNames")).isActive();
		if (!ok)
			qFatal("TRUNCATE TABLE FileNames: %s", qPrintable(m_database.lastError().text()));

		QSqlQuery insertFile(m_database);
		insertFile.prepare(QStringLiteral("INSERT INTO Files (Id) VALUES (?)"));

		storage->filesystem()->enumerateFiles([&insertFile, this](uint64_t key, size_t size) {
			if (key < 0x0100000000ULL) {
				insertFile.bindValue(0, key);
				bool ok = insertFile.exec();
				if(!ok)
					qFatal("INSERT Files: %s", qPrintable(insertFile.lastError().text()));
				insertFile.finish();
			}
		});

		QSqlQuery insertFileName(m_database);
		insertFileName.prepare(QStringLiteral("INSERT INTO FileNames (Id, Name, Harvested) VALUES (?, ?, 0)"));

		storage->filesystem()->enumerateFileNames([&insertFileName, this](const std::string &name, uint64_t key) {
			if (key < 0x0100000000ULL) {
				insertFileName.bindValue(0, key);
				insertFileName.bindValue(1, QString::fromStdString(name));
				bool ok = insertFileName.exec();
				if (!ok)
					qFatal("INSERT FileNames: %s", qPrintable(insertFileName.lastError().text()));
				insertFileName.finish();
			}
		});

		m_database.commit();
	}
}

NameHarvestingEngineInner::~NameHarvestingEngineInner() {
	if (m_transactionOpen) {
		m_database.commit();
	}

	m_database.close();
}

void NameHarvestingEngineInner::harvestStart() {
	if (!m_harvesting) {
		m_harvesting = true;

		harvestingTick();
	}
}

void NameHarvestingEngineInner::harvestStop() {
	if (m_harvesting) {
		m_harvesting = false;
		harvestingTick();
	}
}

void NameHarvestingEngineInner::harvestingTick() {
	if (m_harvesting) {
		bool hasWork = true;

		if (!m_transactionOpen) {
			m_transactionOpen = true;
			if(!m_database.transaction())
				qFatal("transaction: %s", qPrintable(m_database.lastError().text()));
		}

		if (!m_harvestInitialized) {
			m_harvestInitialized = true;

			bool ok = m_database.exec(QStringLiteral("DROP TABLE IF EXISTS FilesToHarvest")).isActive();
			if(!ok)
				qFatal("DROP TABLE(FilesToHarvest): %s", qPrintable(m_database.lastError().text()));

			ok = m_database.exec(QStringLiteral(R"SQL(
				CREATE TEMPORARY TABLE FilesToHarvest(
					Id INTEGER NOT NULL PRIMARY KEY
				)
			)SQL")).isActive();
			if (!ok)
				qFatal("CREATE TABLE(FilesToHarvest): %s", qPrintable(m_database.lastError().text()));

			ok = m_database.exec(QStringLiteral("INSERT INTO FilesToHarvest(Id) SELECT Id FROM Files EXCEPT SELECT Id FROM FileNames")).isActive();
			if (!ok)
				qFatal("INSERT INTO(FilesToHarvest): %s", qPrintable(m_database.lastError().text()));

			auto countQuery = m_database.exec(QStringLiteral("SELECT COUNT(*) FROM FilesToHarvest"));
			if (!countQuery.isActive() || !countQuery.first()) {
				qFatal("SELECT COUNT(FilesToHarvest): %s", qPrintable(m_database.lastError().text()));
			}

			m_filesTotal = countQuery.value(0).toUInt();
			m_filesProcessed = 0;
			emit harvestProgress(m_filesProcessed, m_filesTotal);
		}

		auto query = m_database.exec(QStringLiteral("SELECT Id FROM FilesToHarvest ORDER BY Id LIMIT 100"));
		if(!query.isActive())
			qFatal("SELECT(FilesToHarvest): %s", qPrintable(m_database.lastError().text()));

		m_filesToProcessOnThisTick.clear();

		if (query.first()) {
			do {
				m_filesToProcessOnThisTick.emplace(query.value(0).toULongLong());
			} while (query.next());
		}

		hasWork = !m_filesToProcessOnThisTick.empty();

		while (!m_filesToProcessOnThisTick.empty()) {
			auto id = *m_filesToProcessOnThisTick.begin();
			try {
				m_extractor.extractNamesFromFile(id);
			}
			catch (const std::exception& e) {
				emit processingReject(QStringLiteral("%1: %2").arg(static_cast<qulonglong>(id), 8, 16, QChar('0')).arg(e.what()));
			}
			excludeFile(id);
		}

		emit harvestProgress(m_filesProcessed, m_filesTotal);

		if (!hasWork) {
			if (m_harvesting) {
				m_harvesting = false;
				emit harvestFinished();
			}
		}
	}
	else {
		if (m_transactionOpen) {
			m_transactionOpen = false;
			if(!m_database.commit())
				qFatal("commit: %s", qPrintable(m_database.lastError().text()));

		}
	}

	if (m_harvesting) {
		QTimer::singleShot(0, this, &NameHarvestingEngineInner::harvestingTick);
	}
}

void NameHarvestingEngineInner::excludeFile(uint64_t id) {
	m_filesToProcessOnThisTick.erase(id);

	QSqlQuery query(m_database);
	query.prepare(QStringLiteral("DELETE FROM FilesToHarvest WHERE Id = ?"));
	query.bindValue(0, id);
	if(!query.exec())
		qFatal("DELETE(FilesToHarvest): %s", qPrintable(m_database.lastError().text()));
		
	m_filesProcessed += query.numRowsAffected();
}

void NameHarvestingEngineInner::addFileName(uint64_t id, const std::string& filename) {
	QSqlQuery query(m_database);
	query.prepare(QStringLiteral("INSERT OR IGNORE INTO FileNames (Id, Name, Harvested) VALUES (?, ?, 1)"));
	query.bindValue(0, id);
	query.bindValue(1, QString::fromStdString(filename));
	if (!query.exec())
		qFatal("INSERT INTO(FileNames): %s", qPrintable(query.lastError().text()));
}

