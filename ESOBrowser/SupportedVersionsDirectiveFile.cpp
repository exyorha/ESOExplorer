#include "SupportedVersionsDirectiveFile.h"

SupportedVersionsDirectiveFile::SupportedVersionsDirectiveFile() = default;

SupportedVersionsDirectiveFile::~SupportedVersionsDirectiveFile() = default;

void SupportedVersionsDirectiveFile::processLine(std::vector<std::string>& tokens) {
	if (tokens.front() == "SUPPORTED_VERSION") {
		if (tokens.size() != 2) {
			parseError("SUPPORTED_VERSION: one more token expected");
		}

		supportedVersions.emplace_back(std::move(tokens[1]));
	}
	else {
		parseError("SUPPORTED_VERSION expected");
	}
}
