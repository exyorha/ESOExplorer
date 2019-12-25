#ifndef ESO_DATABASE_RECORD_H
#define ESO_DATABASE_RECORD_H

#include <variant>
#include <unordered_map>
#include <string>

#include "DatabaseDirectiveFile.h"

class ESODatabaseDef;

class ESODatabaseRecord {
public:
	struct ValueEnum {
		const DatabaseDirectiveFile::Enum* definition;
		int32_t value;
	};

	struct ValueArray;
	struct ValueForeignKey;

	using Value = std::variant<std::monostate, unsigned long long, ValueEnum, std::string, ValueArray, ValueForeignKey, bool>;

	struct ValueArray {
		std::vector<Value> values;
	};

	struct ValueForeignKey {
		std::string def;
		uint32_t id;
	};

	ESODatabaseRecord();
	~ESODatabaseRecord();

	ESODatabaseRecord(const ESODatabaseRecord& other);
	ESODatabaseRecord& operator =(const ESODatabaseRecord& other);

	ESODatabaseRecord(ESODatabaseRecord&& other);
	ESODatabaseRecord& operator =(ESODatabaseRecord&& other);

	Value& addField(const std::string& name);
	const Value& findField(const std::string& name) const;

	inline const std::vector<std::string>& fieldOrder() const { return m_fieldOrder; }

private:
	std::unordered_map<std::string, Value> m_fields;
	std::vector<std::string> m_fieldOrder;
};

#endif
