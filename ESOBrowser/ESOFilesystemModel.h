#ifndef ESO_FILESYSTEM_MODEL_H
#define ESO_FILESYSTEM_MODEL_H

#include <QAbstractItemModel>

#include <variant>

namespace esodata {
	class Filesystem;
}

template<>
struct std::hash<QString> {
	size_t operator()(const QString& string) const;
};

class ESOFilesystemModel final : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit ESOFilesystemModel(QObject* parent = nullptr);
	~ESOFilesystemModel() override;

	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent) const override;

	void buildTree(const esodata::Filesystem *fs);
	void sortModel();

	QString nameForId(uint64_t id, bool& isDefault) const;
	QString nameForId(uint64_t id) const;

private:
	struct Inode;
	using Directory = std::vector<std::unique_ptr<Inode>>;
	struct Inode {
		Inode* parent;
		QModelIndex indexOfThis;
		QString name;
		std::variant<uint64_t, Directory> data;
	};

	Inode* makeDirectory(Inode* parent, const QString& name);

	Inode* getContainingDirectory(uint64_t id);
	Inode* getContainingDirectory(const QString &name, QString &nameInDirectory);

	void sortDirectory(Inode* directory);

	Inode m_root;
	Inode* m_byId;
	std::unordered_map<uint64_t, Inode*> m_directoryMap;
	std::unordered_map<uint64_t, QString> m_nameMap;
	std::unordered_map<QString, Inode*> m_namedDirectoryMap;
};

#endif
