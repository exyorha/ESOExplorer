#ifndef ESO_DATABASE_H
#define ESO_DATABASE_H

#include "ESODatabaseDef.h"
#include "ESODatabaseParsingContext.h"

#include <vector>
#include <filesystem>
#include <optional>

namespace esodata {
	class Filesystem;
}

class ESODatabase {
public:
	explicit ESODatabase(const esodata::Filesystem* fs);
	~ESODatabase();
	
	ESODatabase(const ESODatabase& other) = delete;
	ESODatabase& operator =(const ESODatabase& other) = delete;

	inline const std::vector<ESODatabaseDef>& defs() const { return m_defs; }
	inline std::vector<ESODatabaseDef>& defs()  { return m_defs; }

	void loadDirectives(std::filesystem::path& directoryPath);

	const ESODatabaseDef& findDefByName(const std::string& name) const;

private:
	const esodata::Filesystem* m_fs;
	std::vector<ESODatabaseDef> m_defs;
	std::unordered_map<std::string, ESODatabaseDef*> m_defLookupByName;
	std::optional<ESODatabaseParsingContext> m_parsingContext;
};

#endif

