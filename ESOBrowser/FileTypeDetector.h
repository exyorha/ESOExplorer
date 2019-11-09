#ifndef FILE_TYPE_DETECTOR_H
#define FILE_TYPE_DETECTOR_H

#include <vector>

enum class DetectedFileType {
	Unknown,
	Text,
	DDS,
	Granny2
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
	static const unsigned char m_granny2Signature1[16];
	static const unsigned char m_granny2Signature2[16];
};

#endif
