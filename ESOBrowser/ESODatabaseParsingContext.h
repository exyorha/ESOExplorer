#ifndef ESO_DATABASE_PARSING_CONTEXT_H
#define ESO_DATABASE_PARSING_CONTEXT_H

#include <vector>

#include "DatabaseDirectiveFile.h"

struct ESODatabaseParsingContext {
	std::vector<DatabaseDirectiveFile::Structure> defs;
	std::vector<DatabaseDirectiveFile::Structure> structures;
};

#endif
