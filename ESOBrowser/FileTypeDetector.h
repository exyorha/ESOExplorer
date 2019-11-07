#ifndef FILE_TYPE_DETECTOR_H
#define FILE_TYPE_DETECTOR_H

#include <vector>

enum class DetectedFileType {
	Unknown,
	Text,
	DDS
};

class FileTypeDetector {
public:
	FileTypeDetector();
	~FileTypeDetector();

	FileTypeDetector(const FileTypeDetector& other) = delete;
	FileTypeDetector &operator =(const FileTypeDetector& other) = delete;

	DetectedFileType detect(const std::vector<unsigned char>& data) const;

private:
	static const unsigned char m_ddsSignature[4];
};

#endif
