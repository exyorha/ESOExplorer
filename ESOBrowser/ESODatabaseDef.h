#ifndef ESO_DATABASE_DEF_H
#define ESO_DATABASE_DEF_H

#include <string>

#include "ESODatabaseRecord.h"
#include "DatabaseDirectiveFile.h"

namespace esodata {
	class Filesystem;
	class SerializationStream;
}

struct ESODatabaseParsingContext;

class ESODatabaseDef {
public:
	ESODatabaseDef(const esodata::Filesystem *fs, const DatabaseDirectiveFile::Structure& def, const ESODatabaseParsingContext &parsingContext);
	~ESODatabaseDef();

	ESODatabaseDef(const ESODatabaseDef& other) = delete;
	ESODatabaseDef &operator =(const ESODatabaseDef& other) = delete;

	ESODatabaseDef(ESODatabaseDef&& other);
	ESODatabaseDef& operator =(ESODatabaseDef&& other);

	inline unsigned int id() const { return m_id; }
	inline const std::string& name() const { return m_name; }

	void loadDef();

	inline const std::vector<ESODatabaseRecord>& records() const { return m_records; }
	inline std::vector<ESODatabaseRecord>& records() { return m_records; }

private:
	void parseStructureIntoRecord(esodata::SerializationStream& stream, const DatabaseDirectiveFile::Structure& structure, ESODatabaseRecord& record);

	const esodata::Filesystem* m_fs;
	const DatabaseDirectiveFile::Structure* m_def;
	const ESODatabaseParsingContext* m_parsingContext;
	unsigned int m_id;
	std::string m_name;
	std::vector<ESODatabaseRecord> m_records;
};

#endif
