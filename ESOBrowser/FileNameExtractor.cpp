#include "FileNameExtractor.h"
#include "FileTypeDetector.h"
#include <ESOData/Granny2/Granny2TypeHelpers.h>
#include <ESOData/Granny2/ESOGraphicsTypes.h>
#include "FileNameExtractorCallbacks.h"

#include <ESOData/Filesystem/Filesystem.h>

#include <granny.h>

#include <sstream>

FileNameExtractor::FileNameExtractor(const esodata::Filesystem* fs, FileNameExtractorCallbacks* callbacks, const std::vector<std::string>& prefixes) : m_fs(fs), m_callbacks(callbacks), m_prefixes(prefixes),
	m_key(0) {

}

FileNameExtractor::~FileNameExtractor() = default;

void FileNameExtractor::extractNamesFromFile(uint64_t id) {
	m_key = id;

	auto data = m_fs->readFileByKey(id);
	auto type = FileTypeDetector().detect(data);

	switch (type) {
	case DetectedFileType::Granny2:
		extractNamesFromGranny(data);
		break;

	default:
		break;
	}
}

void FileNameExtractor::extractNamesFromGranny(std::vector<unsigned char>& fileData) {
	esodata::GrannyFile file(GrannyReadEntireFileFromMemory(fileData.size(), fileData.data()));
	if (!file) {
		std::stringstream error;
		error << "Failed to read granny2 file " << std::hex << m_key;
		throw std::runtime_error(error.str());
	}

	auto info = GrannyGetFileInfo(file.get());

	for (size_t index = 0, count = info->MaterialCount; index < count; index++) {
		auto material = info->Materials[index];
		if (material->Texture)
			continue;

		if (!material->ExtendedData.Object)
			throw std::logic_error("no Extended Data in material");

		for (size_t mapIndex = 0, mapCount = material->MapCount; mapIndex < mapCount; mapIndex++) {
			const auto& map = material->Maps[mapIndex];
			if (!map.Material || !map.Material->Texture)
				continue;

			const auto& materialData = map.Material->Texture->ExtendedData;

			if (!materialData.Object)
				continue;

			esodata::ESOTexture texture;
			GrannyConvertSingleObject(
				materialData.Type,
				materialData.Object,
				esodata::ESOTextureType,
				&texture,
				nullptr
			);

			if (!texture.fileIndex) {
				throw std::logic_error("no fileIndex in texture data");
			}

			granny_variant field;

			if (!GrannyFindMatchingMember(
				material->ExtendedData.Type,
				material->ExtendedData.Object,
				map.Usage,
				&field
			)) {
				throw std::logic_error("map is not in material: " + std::string(map.Usage));
			}

			if (field.Type->Type != GrannyStringMember)
				throw std::logic_error("string expected");

			auto name = *static_cast<const char**>(field.Object);

			if (*name) {
				processName(texture.fileIndex, name);
			}
			m_callbacks->excludeFile(texture.fileIndex);
		}
	}

	if (info->FromFileName && *info->FromFileName && std::strtoull(info->FromFileName, nullptr, 10) != m_key) {
		processName(m_key, info->FromFileName);
	}
}

void FileNameExtractor::processName(uint64_t key, const char* name) {
	while (*name && *name != '\\')
		name++;

	while (*name && *name == '\\')
		name++;

	for (const auto& prefix : m_prefixes) {
		if (strnicmp(name, prefix.data(), prefix.size()) == 0) {
			std::string outputName = std::string("/art/") + (name + prefix.size());

			for (auto& ch : outputName) {
				if (ch == '\\')
					ch = '/';
			}

			m_callbacks->addFileName(key, outputName);
			return;
		}
	}

	throw std::logic_error("unknown name prefix: " + std::string(name));
}
