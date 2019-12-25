#include "ESODatabaseDef.h"
#include "ESODatabaseParsingContext.h"

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
	m_recordLookup.reserve(itemCount);

	const auto& baseDef = m_parsingContext->findStructureByName("BaseDef");

	for (auto& record : m_records) {
		record.addField("flags").emplace<unsigned long long>(flags);
		record.addField("version").emplace<unsigned long long>(version);

		uint32_t expectedLength;
		stream >> expectedLength;

		std::vector<unsigned char> recordData(expectedLength);
		stream >> esodata::makeDeflatedSegment(recordData);

		esodata::InputSerializationStream contentStream(recordData.data(), recordData.data() + recordData.size());
		contentStream.setSwapEndian(stream.swapEndian());

		parseStructureIntoRecord(contentStream, baseDef, record);
		parseStructureIntoRecord(contentStream, *m_def, record);

		auto id = std::get<unsigned long long>(record.findField("id"));
		m_recordLookup.emplace(id, &record);
	}
}

void ESODatabaseDef::parseStructureIntoRecord(esodata::SerializationStream& stream, const DatabaseDirectiveFile::Structure& structure, ESODatabaseRecord& record) {
	for (const auto& field : structure.fields) {
		switch (field.type) {
		case DatabaseDirectiveFile::FieldType::UInt16:
		{
			uint16_t val;
			stream >> val;
			record.addField(field.name).emplace<unsigned long long>(val);
			break;
		}

		case DatabaseDirectiveFile::FieldType::UInt32:
		{
			uint32_t val;
			stream >> val;
			record.addField(field.name).emplace<unsigned long long>(val);
			break;
		}

		case DatabaseDirectiveFile::FieldType::UInt64:
		{
			uint64_t val;
			stream >> val;
			record.addField(field.name).emplace<unsigned long long>(val);
			break;
		}

		case DatabaseDirectiveFile::FieldType::Enum:
		{
			auto& value = record.addField(field.name).emplace<ESODatabaseRecord::ValueEnum>();
			value.definition = &m_parsingContext->findEnumByName(field.typeName);
			stream >> value.value;
			break;
		}

		case DatabaseDirectiveFile::FieldType::String:
		{
			std::string value;
			stream >> value;
			record.addField(field.name).emplace<std::string>(std::move(value));
			break;
		}
		}
	}
}

const ESODatabaseRecord& ESODatabaseDef::findRecordById(uint64_t id) const {
	auto it = m_recordLookup.find(id);
	if (it == m_recordLookup.end()) {
		throw std::logic_error("Record not found: " + m_name + " " + std::to_string(id));
	}

	return *it->second;
}
