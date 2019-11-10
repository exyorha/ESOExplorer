#ifndef FILE_NAME_EXTRACTOR_H
#define FILE_NAME_EXTRACTOR_H

#include <stdint.h>

#include <vector>
#include <string>

namespace esodata {
	class Filesystem;
}

class FileNameExtractorCallbacks;

class FileNameExtractor {
public:
	FileNameExtractor(const esodata::Filesystem* fs, FileNameExtractorCallbacks* callbacks, const std::vector<std::string> &prefixes);
	~FileNameExtractor();

	FileNameExtractor(const FileNameExtractor& other) = delete;
	FileNameExtractor &operator =(const FileNameExtractor& other) = delete;

	void extractNamesFromFile(uint64_t id);

private:
	void extractNamesFromGranny(std::vector<unsigned char>& data);

	void processName(uint64_t key, const char* name);

	const esodata::Filesystem* m_fs;
	FileNameExtractorCallbacks* m_callbacks;
	const std::vector<std::string>& m_prefixes;
	uint64_t m_key;
};

#endif
