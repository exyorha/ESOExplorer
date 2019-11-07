#ifndef FILESYSTEM_DIRECTIVE_FILE_H
#define FILESYSTEM_DIRECTIVE_FILE_H

#include "DirectiveFile.h"

#include <string>

class FilesystemDirectiveFile final : public DirectiveFile {
public:
	FilesystemDirectiveFile();
	~FilesystemDirectiveFile();

	std::vector<std::string> manifests;
	std::vector<uint64_t> fileTables;

private:
	void processLine(std::vector<std::string>& tokens) override;
};


#endif
