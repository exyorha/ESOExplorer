#include "ESODatabaseDef.h"

#include <ESOData/Filesystem/Filesystem.h>
#include <ESOData/Serialization/InputSerializationStream.h>
#include <ESOData/Serialization/DeflatedSegment.h>

#include <sstream>

ESODatabaseDef::ESODatabaseDef(const esodata::Filesystem* fs, const DatabaseDirectiveFile::Structure& def, const ESODatabaseParsingContext& parsingContext) :
	m_id(def.defIndex),
	m_name(def.name),
	m_fs(fs),
	m_def(&def),
	m_parsingContext(&parsingContext) {

}

ESODatabaseDef::~ESODatabaseDef() = default;

ESODatabaseDef::ESODatabaseDef(ESODatabaseDef&& other) = default;

ESODatabaseDef& ESODatabaseDef::operator =(ESODatabaseDef&& other) = default;

void ESODatabaseDef::loadDef() {
	auto defData = m_fs->readFileByKey(0x6000000000000000U | m_id);

	esodata::InputSerializationStream stream(defData.data(), defData.data() + defData.size());
	stream.setSwapEndian(true);

	uint32_t flags = 0;
	uint32_t itemCount;
	stream >> itemCount;

	if (itemCount == 0xFAFAEBEBU) {
		stream >> flags;
		stream >> itemCount;
	}

	if (flags != 0x13)
		throw std::logic_error("flag value of 0x13 is expected");

	uint32_t version;
	stream >> version;
	
	if (m_def->version != 0 && m_def->version != version) {
		std::stringstream error;
		error << "Def file " << m_name << " (" << m_id << ") has unsupported version " << version << " (expected " << version << ").";
		throw std::runtime_error(error.str());
	}

	m_records.resize(itemCount);

	for (auto& record : m_records) {
		record.addField("flags").emplace<unsigned long long>(flags);
		record.addField("version").emplace<unsigned long long>(version);

		uint32_t expectedLength;
		stream >> expectedLength;

		std::vector<unsigned char> recordData(expectedLength);
		stream >> esodata::makeDeflatedSegment(recordData);


	}
}
