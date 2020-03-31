#ifndef ESO_DATABASE_PARSING_CONTEXT_H
#define ESO_DATABASE_PARSING_CONTEXT_H

#include <vector>

#include "DatabaseDirectiveFile.h"

struct ESODatabaseParsingContext {
	ESODatabaseParsingContext() = default;

	ESODatabaseParsingContext(const ESODatabaseParsingContext& other) = delete;
	ESODatabaseParsingContext &operator =(const ESODatabaseParsingContext& other) = delete;

	std::vector<DatabaseDirectiveFile::Structure> defs;
	std::vector<DatabaseDirectiveFile::Structure> structures;
	std::vector<DatabaseDirectiveFile::Enum> enums;
	std::vector<DatabaseDirectiveFile::DefAlias> defAliases;

	void buildLookupCaches();

	const DatabaseDirectiveFile::Structure& findStructureByName(const std::string& name) const;
	const DatabaseDirectiveFile::Structure& findDefByName(const std::string& name) const;
	const DatabaseDirectiveFile::Enum& findEnumByName(const std::string& name) const;

private:
	std::unordered_map<std::string, const DatabaseDirectiveFile::Structure*> m_structureLookup;
	std::unordered_map<std::string, const DatabaseDirectiveFile::Structure*> m_defLookup;
	std::unordered_map<std::string, const DatabaseDirectiveFile::Enum*> m_enumLookup;
	std::unordered_map<std::string, const DatabaseDirectiveFile::DefAlias*> m_defAliasLookup;
};

#endif
