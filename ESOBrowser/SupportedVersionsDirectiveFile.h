#ifndef SUPPORTED_VERSIONS_DIRECTIVE_FILE_H
#define SUPPORTED_VERSIONS_DIRECTIVE_FILE_H

#include "DirectiveFile.h"

#include <vector>
#include <string>

class SupportedVersionsDirectiveFile final : public DirectiveFile {
public:
	SupportedVersionsDirectiveFile();
	~SupportedVersionsDirectiveFile();

	std::vector<std::string> supportedVersions;

protected:
	void processLine(std::vector<std::string>& tokens) override;
};

#endif
