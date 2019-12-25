#include "ESODatabaseRecord.h"

ESODatabaseRecord::ESODatabaseRecord() = default;

ESODatabaseRecord::~ESODatabaseRecord() = default;

ESODatabaseRecord::ESODatabaseRecord(const ESODatabaseRecord& other) = default;

ESODatabaseRecord& ESODatabaseRecord::operator =(const ESODatabaseRecord& other) = default;

ESODatabaseRecord::ESODatabaseRecord(ESODatabaseRecord&& other) = default;

ESODatabaseRecord& ESODatabaseRecord::operator =(ESODatabaseRecord&& other) = default;

auto ESODatabaseRecord::addField(const std::string& name) -> Value & {
	std::string realName = name;

	if (name.empty()) {
		realName = "unk_" + std::to_string(m_fields.size() + 1);
	}

	auto result = m_fields.emplace(realName, std::monostate());
	if (result.second) {
		m_fieldOrder.emplace_back(realName);
	}

	return result.first->second;
}

auto ESODatabaseRecord::findField(const std::string& name) -> const Value& {
	auto result = m_fields.find(name);
	if (result == m_fields.end())
		throw std::logic_error("Required field was not found: " + name);

	return result->second;
}
