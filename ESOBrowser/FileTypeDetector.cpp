#include "FileTypeDetector.h"

#include <QImageReader>
#include <QBuffer>

const unsigned char FileTypeDetector::m_ddsSignature[4]{
	'D', 'D', 'S', ' '
};

const unsigned char FileTypeDetector::m_granny2Signature1[16]{
	0xe5, 0x9b, 0x49, 0x5e, 0x6f, 0x63, 0x1f, 0x14, 0x1e, 0x13, 0xeb, 0xa9, 0x90, 0xbe, 0xed, 0xc4
};

const unsigned char FileTypeDetector::m_granny2Signature2[16]{
	0x29, 0xde, 0x6c, 0xc0, 0xba, 0xa4, 0x53, 0x2b, 0x25, 0xf5, 0xb7, 0xa5, 0xf6, 0x66, 0xe2, 0xee
};

FileTypeDetector::FileTypeDetector() = default;

FileTypeDetector::~FileTypeDetector() = default;

DetectedFileType FileTypeDetector::detect(const std::vector<unsigned char>& data) const {
	if (data.size() >= sizeof(m_ddsSignature) && memcmp(data.data(), m_ddsSignature, sizeof(m_ddsSignature)) == 0) {
		return DetectedFileType::DDS;
	}

	if (data.size() >= sizeof(m_granny2Signature1) && memcmp(data.data(), m_granny2Signature1, sizeof(m_granny2Signature1)) == 0) {
		return DetectedFileType::Granny2;
	}

	if (data.size() >= sizeof(m_granny2Signature2) && memcmp(data.data(), m_granny2Signature2, sizeof(m_granny2Signature2)) == 0) {
		return DetectedFileType::Granny2;
	}

	for (unsigned char byte : data) {
		if (byte == 0x01 || byte >= 0x7F)
			return DetectedFileType::Unknown;
	}

	return DetectedFileType::Text;
}
