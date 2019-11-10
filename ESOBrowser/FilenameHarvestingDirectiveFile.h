#ifndef FILENAME_HARVESTING_DIRECTIVE_FILE_H
#define FILENAME_HARVESTING_DIRECTIVE_FILE_H

#include "DirectiveFile.h"

class FilenameHarvestingDirectiveFile final: public DirectiveFile {
public:
	FilenameHarvestingDirectiveFile();
	~FilenameHarvestingDirectiveFile();

	std::vector<std::string> prefixes;

protected:
	void processLine(std::vector<std::string>& tokens) override;
};

#endif
