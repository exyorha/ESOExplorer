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
	}

	std::sort(parsingContext.defs.begin(), parsingContext.defs.end(), [](const DatabaseDirectiveFile::Structure & a, const DatabaseDirectiveFile::Structure& b) {
		return a.defIndex < b.defIndex;
	});

	m_defs.reserve(parsingContext.defs.size());

	for (const auto& def : parsingContext.defs) {
		m_defs.emplace_back(m_fs, def, parsingContext);
	}
}
