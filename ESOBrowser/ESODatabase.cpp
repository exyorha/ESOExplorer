#include "ESODatabase.h"
#include "DatabaseDirectiveFile.h"
#include "ESODatabaseParsingContext.h"

ESODatabase::ESODatabase(const esodata::Filesystem* fs) : m_fs(fs) {

}

ESODatabase::~ESODatabase() = default;


void ESODatabase::loadDirectives(std::filesystem::path& directoryPath) {
	m_parsingContext.emplace();

	auto& parsingContext = *m_parsingContext;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
		if (!entry.is_regular_file())
			continue;

		DatabaseDirectiveFile directives;
		directives.parseFile(entry);

		parsingContext.structures.insert(parsingContext.structures.end(), std::make_move_iterator(directives.structures().begin()), std::make_move_iterator(directives.structures().end()));
		parsingContext.defs.insert(parsingContext.defs.end(), std::make_move_iterator(directives.defs().begin()), std::make_move_iterator(directives.defs().end()));
		parsingContext.enums.insert(parsingContext.enums.end(), std::make_move_iterator(directives.enums().begin()), std::make_move_iterator(directives.enums().end()));
		parsingContext.defAliases.insert(parsingContext.defAliases.end(), std::make_move_iterator(directives.defAliases().begin()), std::make_move_iterator(directives.defAliases().end()));
	}

	parsingContext.defs.reserve(parsingContext.defs.size() + parsingContext.defAliases.size());

	std::sort(parsingContext.defs.begin(), parsingContext.defs.end(), [](const DatabaseDirectiveFile::Structure & a, const DatabaseDirectiveFile::Structure& b) {
		return a.defIndex < b.defIndex;
	});
	
	parsingContext.buildLookupCaches();

	for (const auto& alias : parsingContext.defAliases) {
		auto& def = parsingContext.defs.emplace_back();

		def.defIndex = alias.defIndex;
		def.name = alias.name;

		const auto& src = parsingContext.findDefByName(alias.targetName);

		def.version = src.version;
		def.fields = src.fields;
	}

	std::sort(parsingContext.defs.begin(), parsingContext.defs.end(), [](const DatabaseDirectiveFile::Structure& a, const DatabaseDirectiveFile::Structure& b) {
		return a.defIndex < b.defIndex;
	});

	parsingContext.buildLookupCaches();

	m_defs.reserve(parsingContext.defs.size());
	
	for (const auto& def : parsingContext.defs) {
		m_defs.emplace_back(m_fs, def, parsingContext);
	}

	for (auto& def : m_defs) {
		m_defLookupByName.emplace(def.name(), &def);
	}
}

const ESODatabaseDef& ESODatabase::findDefByName(const std::string& name) const {
	auto it = m_defLookupByName.find(name);
	if (it == m_defLookupByName.end()) {
		throw std::logic_error("Def not found: " + name);
	}

	return *it->second;
}
