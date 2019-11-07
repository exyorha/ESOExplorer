#include "FilesystemDirectiveFile.h"

FilesystemDirectiveFile::FilesystemDirectiveFile() = default;

FilesystemDirectiveFile::~FilesystemDirectiveFile() = default;

void FilesystemDirectiveFile::processLine(std::vector<std::string>& tokens) {
	if (tokens.front() == "MANIFEST") {
		if (tokens.size() != 2) {
			parseError("MANIFEST: one more token expected");
		}

		manifests.emplace_back(std::move(tokens[1]));
	} else if (tokens.front() == "FILE_TABLE") {
		if (tokens.size() != 2) {
			parseError("FILE_TABLE: one more token expected");
		}

		fileTables.emplace_back(std::stoull(tokens[1], nullptr, 0));
	}
	else {
		parseError("SUPPORTED_VERSION expected");
	}
}
