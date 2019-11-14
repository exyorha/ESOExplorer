#include "ESODatabaseRecord.h"

ESODatabaseRecord::ESODatabaseRecord() = default;

ESODatabaseRecord::~ESODatabaseRecord() = default;

ESODatabaseRecord::ESODatabaseRecord(const ESODatabaseRecord& other) = default;

ESODatabaseRecord& ESODatabaseRecord::operator =(const ESODatabaseRecord& other) = default;

ESODatabaseRecord::ESODatabaseRecord(ESODatabaseRecord&& other) = default;

ESODatabaseRecord& ESODatabaseRecord::operator =(ESODatabaseRecord&& other) = default;

auto ESODatabaseRecord::addField(const std::string& name) -> Value & {
	auto result = m_fields.emplace(name, std::monostate());
	if (result.second) {
		m_fieldOrder.emplace_back(name);
	}

	return result.first->second;
}
