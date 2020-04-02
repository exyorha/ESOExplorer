#include "ESOFilesystemModel.h"

#include <ESOData/Filesystem/Filesystem.h>

ESOFilesystemModel::ESOFilesystemModel(QObject* parent) : QAbstractItemModel(parent), m_byId(nullptr) {

}

ESOFilesystemModel::~ESOFilesystemModel() = default;

void ESOFilesystemModel::buildTree(const esodata::Filesystem* fs) {
	m_root.indexOfThis = QModelIndex();
	m_root.parent = nullptr;
	m_root.data.emplace<Directory>();

	m_byId = makeDirectory(&m_root, QStringLiteral("by-id"));

	fs->enumerateFileNames([this](const std::string& name, uint64_t key) {
		m_nameMap.emplace(key, QString::fromStdString(name));
	});

	fs->enumerateFiles([this](uint64_t key, size_t size) {
		(void)size;

		auto parent = getContainingDirectory((0x07ULL << 56) | (key >> 8));

		auto& directory = std::get<Directory>(parent->data);

		auto entry = std::make_unique<Inode>();
		entry->parent = parent;
		entry->indexOfThis = createIndex(static_cast<int>(directory.size()), 0, entry.get());
		entry->name = QString().setNum(key, 16).rightJustified(16, '0');
		entry->data.emplace<uint64_t>(key);
		directory.emplace_back(std::move(entry));

		auto nameIt = m_nameMap.find(key);
		if (nameIt != m_nameMap.end()) {
			QString nameInDirectory;
			auto parent = getContainingDirectory(nameIt->second, nameInDirectory);
			auto& directory = std::get<Directory>(parent->data);

			auto entry = std::make_unique<Inode>();
			entry->parent = parent;
			entry->indexOfThis = createIndex(static_cast<int>(directory.size()), 0, entry.get());
			entry->name = nameInDirectory;
			entry->data.emplace<uint64_t>(key);
			directory.emplace_back(std::move(entry));
		}
	});

	m_directoryMap.clear();
	m_namedDirectoryMap.clear();
}

QString ESOFilesystemModel::nameForId(uint64_t id) const {
	bool unused;
	return nameForId(id, unused);
}

QString ESOFilesystemModel::nameForId(uint64_t id, bool& isDefault) const {
	auto it = m_nameMap.find(id);
	if (it == m_nameMap.end()) {
		isDefault = true;
		return QStringLiteral("/by-id/") + QString().setNum(id, 16).rightJustified(16, '0');
	}
	else {
		isDefault = false;
		return it->second;
	}
}

void ESOFilesystemModel::sortModel() {
	sortDirectory(&m_root);
}

void ESOFilesystemModel::sortDirectory(Inode* inode) {
	auto& directory = std::get<Directory>(inode->data);

	std::sort(directory.begin(), directory.end(), [](const std::unique_ptr<Inode>& a, const std::unique_ptr<Inode>& b) -> bool {
		return a->name < b->name;
	});

	for (auto& entry : directory) {
		auto childDirectory = std::get_if<Directory>(&entry->data);

		if (childDirectory)
			sortDirectory(entry.get());
	}
		
}

auto ESOFilesystemModel::getContainingDirectory(uint64_t id) -> Inode* {
	auto it = m_directoryMap.find(id);
	if (it != m_directoryMap.end())
		return it->second;

	auto level = id >> 56;

	Inode* parent;
	if (level == 1) {
		parent = m_byId;
	}
	else {
		parent = getContainingDirectory(((level - 1) << 56) | ((id & 0x00FFFFFFFFFFFFFF) >> 8));
	}

	QString name = QString().setNum((id & 0xFF), 16).rightJustified(2, '0');

	auto dir = makeDirectory(parent, name);

	m_directoryMap.emplace(id, dir);

	return dir;
}

auto ESOFilesystemModel::makeDirectory(Inode* parent, const QString& name) -> Inode * {
	auto& directory = std::get<Directory>(parent->data);
	
	auto entry = std::make_unique<Inode>();
	entry->parent = parent;
	entry->indexOfThis = createIndex(static_cast<int>(directory.size()), 0, entry.get());
	entry->name = name;
	entry->data.emplace<Directory>();

	auto entryPtr = entry.get();

	directory.emplace_back(std::move(entry));

	return entryPtr;
}

int ESOFilesystemModel::columnCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return 1;
}

QVariant ESOFilesystemModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QModelIndex();

	auto inode = static_cast<Inode*>(index.internalPointer());

	if (role == Qt::DisplayRole)
		return inode->name;
	else if (role == Qt::UserRole) {
		auto index = std::get_if<uint64_t>(&inode->data);
		if (index) {
			return *index;
		}
		else {
			return QVariant();
		}
	}
	else
		return QVariant();
}

QModelIndex ESOFilesystemModel::index(int row, int column, const QModelIndex& parent) const {
	if (column > 0)
		return QModelIndex();

	auto inode = &m_root;

	if (parent.isValid()) {
		inode = static_cast<Inode*>(parent.internalPointer());
	}

	auto directory = std::get_if<Directory>(&inode->data);
	if (directory) {
		if (row >= directory->size())
			return QModelIndex();

		return createIndex(row, column, (*directory)[row].get());
	}
	else {
		return QModelIndex();
	}
}

QModelIndex ESOFilesystemModel::parent(const QModelIndex& index) const {
	if (!index.isValid()) {
		return QModelIndex();
	}
	else {
		auto inode = static_cast<Inode*>(index.internalPointer());

		if (inode->parent) {
			return inode->parent->indexOfThis;
		}
		else {
			return QModelIndex();
		}
	}
}

int ESOFilesystemModel::rowCount(const QModelIndex& parent) const {
	auto inode = &m_root;

	if (parent.isValid()) {
		inode = static_cast<Inode*>(parent.internalPointer());
	}

	auto directory = std::get_if<Directory>(&inode->data);
	if (directory) {
		return static_cast<int>(directory->size());
	}
	else {
		return 0;
	}
}

auto ESOFilesystemModel::getContainingDirectory(const QString& name, QString& nameInDirectory) -> Inode* {
	auto lastDelimiter = name.lastIndexOf('/');
	if (lastDelimiter < 0) {
		nameInDirectory = name;
		return &m_root;
	}

	auto directoryName = name.mid(0, lastDelimiter);
	nameInDirectory = name.mid(lastDelimiter + 1);

	if (directoryName.isEmpty())
		return &m_root;

	auto it = m_namedDirectoryMap.find(directoryName);
	if (it != m_namedDirectoryMap.end()) {
		return it->second;
	}

	QString myName;
	auto container = getContainingDirectory(directoryName, myName);
	auto me = makeDirectory(container, myName);

	m_namedDirectoryMap.emplace(std::move(directoryName), me);

	return me;
}

size_t std::hash<QString>::operator()(const QString& string) const {
	return qHash(string);
}
