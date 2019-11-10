#ifndef FILE_NAME_EXTRACTOR_CALLBACKS_H
#define FILE_NAME_EXTRACTOR_CALLBACKS_H

#include <stdint.h>

#include <string>

class FileNameExtractorCallbacks {
protected:
	FileNameExtractorCallbacks();
	~FileNameExtractorCallbacks();

public:
	FileNameExtractorCallbacks(const FileNameExtractorCallbacks& other) = delete;
	FileNameExtractorCallbacks &operator =(const FileNameExtractorCallbacks& other) = delete;

	virtual void addFileName(uint64_t id, const std::string& name) = 0;
	virtual void excludeFile(uint64_t id) = 0;
};

#endif
