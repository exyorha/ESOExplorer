#include "FilenameHarvestingDirectiveFile.h"

FilenameHarvestingDirectiveFile::FilenameHarvestingDirectiveFile() = default;

FilenameHarvestingDirectiveFile::~FilenameHarvestingDirectiveFile() = default;

void FilenameHarvestingDirectiveFile::processLine(std::vector<std::string>& tokens) {
	if (tokens.front() == "PREFIX") {
		if (tokens.size() != 2) {
			parseError("PREFIX: one more token expected");
		}

		prefixes.emplace_back(std::move(tokens[1]));
	}
	else {
		parseError("PREFIX expected");
	}
}

