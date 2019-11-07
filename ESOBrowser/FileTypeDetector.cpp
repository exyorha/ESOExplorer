#include "FileTypeDetector.h"

#include <QImageReader>
#include <QBuffer>

const unsigned char FileTypeDetector::m_ddsSignature[4]{
	'D', 'D', 'S', ' '
};

FileTypeDetector::FileTypeDetector() = default;

FileTypeDetector::~FileTypeDetector() = default;

DetectedFileType FileTypeDetector::detect(const std::vector<unsigned char>& data) const {
	if (data.size() >= sizeof(m_ddsSignature) && memcmp(data.data(), m_ddsSignature, sizeof(m_ddsSignature)) == 0) {
		return DetectedFileType::DDS;
	}

	for (unsigned char byte : data) {
		if (byte == 0x01 || byte >= 0x7F)
			return DetectedFileType::Unknown;
	}

	return DetectedFileType::Text;
}
